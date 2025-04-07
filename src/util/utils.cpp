#include <iostream>

#include "utils.h"

std::string PATH;
char *SERVER_IP = "127.0.0.1";
bool IS_SERVER = false;

bool parse_init(int argc, char *argv[]) {
  switch (argc) {
  case 3: {
    SERVER_IP = argv[2];
    IS_SERVER = true;
  }
  case 2: {
    PATH = argv[1];
    return true;
  }

  default: {
    std::cout << "Usage: ./live-vim <path to file> [server ip]" << std::endl;
    return false;
  }
  }
}