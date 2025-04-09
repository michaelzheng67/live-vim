#ifndef __NETWORK_H_
#define __NETWORK_H_

#include <boost/interprocess/ipc/message_queue.hpp>

typedef enum { INSERT, DELETE, SHUTDOWN } op;

using namespace boost::interprocess;

// representation of bytes sent over the wire
struct packet {
  op operation;
  int x;
  int y;
  char c;
};

void server_loop(const unsigned int server_port, message_queue &send_q,
                 message_queue &receive_q,
                 char *debug_file_path = "./data/output.txt");
void client_loop(const char *server_ip, const unsigned int server_port,
                 message_queue &send_q, message_queue &receive_q,
                 char *debug_file_path = "./data/output.txt");

#endif