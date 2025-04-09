#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <boost/interprocess/ipc/message_queue.hpp>
#include <ncurses.h>
#include <unistd.h>

#include "argparse/CLI11.hpp"
#include "buffer/buffer.h"
#include "io/io_utils.h"
#include "network/network.h"

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
typedef enum { INVALID_MODE, QUIT_MODE, INSERT_MODE, SAVE_MODE } MODE;

std::vector<bool> modes{false, false, false};

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

  else if (vec.size() == 3 && vec[0] == ':' && vec[1] == 'w' && vec[2] == 'q') {
    modes[SAVE_MODE] = true;
    vec.clear();
  }

  else {
    modes[INVALID_MODE] = true;
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

void print_screen(int x, int y, std::string &queued_cmd) {
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  clear();
  printw(BUF.get_str_repr().c_str());

  if (modes[INSERT_MODE]) {
    mvprintw(rows - 1, 0, "-- Insert --");
  }

  if (modes[INVALID_MODE]) {
    attron(COLOR_PAIR(1));
    mvprintw(rows - 1, 0, "Not an editor command: %s",
             queued_cmd.substr(1).c_str());
    attroff(COLOR_PAIR(1));

    queued_cmd.clear();
    modes[INVALID_MODE] = false;
  }

  if (!mode_toggled(modes)) {
    mvprintw(rows - 1, 0, queued_cmd.c_str());
  }

  std::string coordinates = std::to_string(y) + "," + std::to_string(x);
  mvprintw(rows - 1, cols - coordinates.size(), "%s", coordinates.c_str());

  move(y, x);
  refresh();
}

void terminal_gui_loop(const char *file_path, message_queue &send_q,
                       message_queue &receive_q) {
  initscr();
  start_color();
  use_default_colors();
  init_pair(1, -1, COLOR_RED);

  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  create_buf(file_path);

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
        print_screen(x, y, queued_cmd);
        break;
      case DELETE:
        BUF.delete_at(packet.x, packet.y);
        print_screen(x, y, queued_cmd);
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
          send_delete_command(send_q, x, y);
          std::tie(x, y) = BUF.delete_at(x, y);
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
          send_insert_command(send_q, x, y, c);
          std::tie(x, y) = BUF.insert_at(x, y, c);
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
          send_insert_command(send_q, x, y, c);
          std::tie(x, y) = BUF.insert_at(x, y, c);
        } else {
          queued_cmd.push_back(c);

          // special case - insert mode doesn't require user to press 'enter'
          eval_insert_command(queued_cmd);
        }
        break;
      }

      if (modes[QUIT_MODE] || modes[SAVE_MODE]) {
        goto exit;
      }

      adjust_x(x, BUF, y);
      adjust_y(y, BUF);

      print_screen(x, y, queued_cmd);
    }
  }

exit:

  endwin();

  if (modes[SAVE_MODE]) {
    write_out_buf(file_path, BUF.get_str_repr());
  }

  // invariant: network process always checks send queue for shutdown command
  send_shutdown_command(send_q);
}

int main(int argc, char **argv) {

  // parse user arguments
  CLI::App app{"Vim with your friends 📝"};
  argv = app.ensure_utf8(argv);

  std::string file_path = "default";
  app.add_option("-f,--file", file_path, "Path to file you want to edit");

  std::string server_ip = "127.0.0.1";
  app.add_option("--server_ip", server_ip, "Server IP to connect to");

  int server_port = 8002;
  app.add_option("--server_port", server_port, "Server port to connect to");

  bool is_server = false;
  app.add_flag("-s,--server", is_server, "Make this a server process");

  CLI11_PARSE(app, argc, argv);

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
    if (is_server) {
      server_loop(server_port, send_q, receive_q);
    } else {
      try {
        client_loop(server_ip.c_str(), server_port, send_q, receive_q);
      } catch (std::exception &e) {

        // in case network process dies, shut down ncursor process
        packet shutdown{SHUTDOWN};
        receive_q.send(&shutdown, sizeof(shutdown), 0);
        return 0;
      }
    }

    // parent process
  } else {
    terminal_gui_loop(file_path.c_str(), send_q, receive_q);
  }

  // cleanup
  message_queue::remove(TO_SEND);
  message_queue::remove(TO_RECEIVE);
}