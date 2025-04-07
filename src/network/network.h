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

void server_loop(message_queue &send_q, message_queue &receive_q);
void client_loop(const char *server_ip, message_queue &send_q,
                 message_queue &receive_q);

#endif