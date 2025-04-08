#include <iostream>

#include "utils.h"

std::string PATH;
char *SERVER_IP = "127.0.0.1";
int SERVER_PORT = 9002;
bool IS_SERVER = false;

bool parse_init(int argc, char *argv[]) {
  switch (argc) {
  case 5: {
    IS_SERVER = true;
  }
  case 4: {
    SERVER_PORT = std::stoi(argv[3]);
  }
  case 3: {
    SERVER_IP = argv[2];
  }
  case 2: {
    PATH = argv[1];
    return true;
  }

  default: {
    std::cout << "Usage: ./live-vim <path to file> [server ip] [server port] "
                 "[any arg if is server instance]"
              << std::endl;
    return false;
  }
  }
}