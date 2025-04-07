#ifndef __UTILS_H_
#define __UTILS_H_

#include <string>

extern std::string PATH;
extern char *SERVER_IP;
extern bool IS_SERVER;

// parses user input and stores them in global variables
bool parse_init(int argc, char *argv[]);

#endif