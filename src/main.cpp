#include <fstream>
#include <iostream>
#include <string>
#include <thread>

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
      case 'q':
        goto exit;
      case BACKSPACE:
        x--;
        BUF.delete_at(x, y);
        send_delete_command(send_q, x, y);
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
      default:
        BUF.insert_at(x, y, c);
        send_insert_command(send_q, x, y, c);
        x++;
        break;
      }

      adjust_x(x, BUF, y);
      adjust_y(y, BUF);

      clear();
      printw(BUF.get_str_repr().c_str());
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