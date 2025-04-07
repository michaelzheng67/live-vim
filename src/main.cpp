#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <ncurses.h>
#include <unistd.h>

#include "buffer/buffer.h"
#include "io/io_utils.h"
#include "network/network.h"
#include "util/utils.h"

using namespace boost::interprocess;

#define TO_SEND "to_send"
#define TO_RECEIVE "to_receive"

void send_insert_command(message_queue &send_q, int x, int y, char c) {
  packet packet{INSERT, x, y, c};
  send_q.send(&packet, sizeof(packet), 0);
}

void send_delete_command(message_queue &send_q, int x, int y) {
  packet packet{DELETE, x, y};
  send_q.send(&packet, sizeof(packet), 0);
}

void send_shutdown_command(message_queue &send_q) {
  packet packet{SHUTDOWN};
  send_q.send(&packet, sizeof(packet), 0);
}

// indexes into modes vector
typedef enum { QUIT_MODE, INSERT_MODE } MODE;

std::vector<bool> modes{false, false};

bool quit_mode(std::vector<bool> &modes, char f, char s) {
  for (int i = 0; i < modes.size(); i++) {
    if (modes[i] == true) {
      return false;
    }
  }

  return f == ':' && (s == 'q' || s == 'Q');
}

bool insert_mode(char f) { return (f == 'i' || f == 'I'); }

bool escaped_mode(unsigned int f) { return f == 27; }

bool user_typing_mode(char f) { return f == ':'; }

void eval_command(std::string &vec) {
  // toggles index within modes array so that we can switch on a given mode for
  // the next iteration

  if (vec.size() == 2 && vec[0] == ':' && vec[1] == 'q') {
    modes[QUIT_MODE] = true;
    vec.clear();
  }
}

void eval_insert_command(std::string &vec) {
  if (vec.size() == 1 && vec[0] == 'i') {
    modes[INSERT_MODE] = true;
    vec.clear();
  }
}

void clear_all_modes(std::vector<bool> &vec) {
  for (int i = 0; i < vec.size(); i++) {
    vec[i] = false;
  }
}

bool mode_toggled(std::vector<bool> &vec) {
  for (int i = 0; i < vec.size(); i++) {
    if (vec[i] == true) {
      return true;
    }
  }
  return false;
}

void terminal_gui_loop(message_queue &send_q, message_queue &receive_q) {
  initscr(); // Start curses mode
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  create_buf(PATH);

  printw(BUF.get_str_repr().c_str());

  int c;
  int x = 0;
  int y = 0;

  // state needed for shortcuts + modes
  std::string queued_cmd;

  move(y, x);

  // busy loop
  while (true) {

    // nonblocking packet receive
    message_queue::size_type recvd_size;
    unsigned int priority;
    packet packet;

    if (receive_q.try_receive(&packet, sizeof(packet), recvd_size, priority)) {

      switch (packet.operation) {
      case INSERT:
        BUF.insert_at(packet.x, packet.y, packet.c);
        clear();
        printw(BUF.get_str_repr().c_str());
        move(y, x);
        refresh();
        break;
      case DELETE:
        BUF.delete_at(packet.x, packet.y);
        clear();
        printw(BUF.get_str_repr().c_str());
        move(y, x);
        refresh();
        break;
      case SHUTDOWN:
        goto exit;
      default:
        break;
      }
    }

    // nonblocking screen update
    if ((c = getch()) != ERR) {
      switch (c) {
      case BACKSPACE:
        if (modes[INSERT_MODE]) {
          x--;
          BUF.delete_at(x, y);
          send_delete_command(send_q, x, y);
        } else {
          queued_cmd.pop_back();
        }

        break;
      case KEY_LEFT:
        x--;
        break;
      case KEY_RIGHT:
        x++;
        break;
      case KEY_UP:
        y--;
        break;
      case KEY_DOWN:
        y++;
        break;
      case '\n':
        if (modes[INSERT_MODE]) {
          BUF.insert_at(x, y, c);
          send_insert_command(send_q, x, y, c);
          y++;
          x = 0;
        }

        if (!mode_toggled(modes)) {
          eval_command(queued_cmd);
        }
        break;
      case '\033':
        clear_all_modes(modes);
        break;
      default:
        if (modes[INSERT_MODE]) {
          BUF.insert_at(x, y, c);
          send_insert_command(send_q, x, y, c);
          x++;
        } else {
          queued_cmd.push_back(c);

          // special case - insert mode doesn't require user to press 'enter'
          eval_insert_command(queued_cmd);
        }
        break;
      }

      if (modes[QUIT_MODE]) {
        goto exit;
      }

      adjust_x(x, BUF, y);
      adjust_y(y, BUF);

      clear();
      printw(BUF.get_str_repr().c_str());

      if (modes[INSERT_MODE]) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        mvprintw(rows - 1, 0, "-- Insert --");
      }

      if (!mode_toggled(modes)) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        mvprintw(rows - 1, 0, queued_cmd.c_str());
      }

      move(y, x);
      refresh();
    }
  }

exit:

  endwin();
  write_out_buf(PATH, BUF.get_str_repr());

  // invariant: network process always checks send queue for shutdown command
  send_shutdown_command(send_q);
}

int main(int argc, char *argv[]) {

  // parse user arguments
  bool res = parse_init(argc, argv);
  if (!res) {
    return 0;
  }

  // sanity check that previous MQs were cleaned up
  message_queue::remove(TO_SEND);
  message_queue::remove(TO_RECEIVE);

  // create queues for IPC
  message_queue send_q(create_only, TO_SEND, 100, sizeof(packet));
  message_queue receive_q(create_only, TO_RECEIVE, 100, sizeof(packet));

  pid_t pid = fork();

  // Note: ncurses is not thread safe, thus we must spawn child processes to do
  // concurrent logic.

  // child process
  if (pid == 0) {
    if (IS_SERVER) {
      print("server");
      server_loop(send_q, receive_q);
    } else {
      try {
        client_loop(SERVER_IP, send_q, receive_q);
      } catch (std::exception &e) {

        // in case network process dies, shut down ncursor process
        packet shutdown{SHUTDOWN};
        receive_q.send(&shutdown, sizeof(shutdown), 0);
        return 0;
      }
    }

    // parent process
  } else {
    terminal_gui_loop(send_q, receive_q);
  }

  // cleanup
  message_queue::remove(TO_SEND);
  message_queue::remove(TO_RECEIVE);
}