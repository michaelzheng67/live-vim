#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <string>

extern std::string BUF;

void create_buf(const std::string& path);

void write_out_buf(const std::string& path, const std::string& buf);

#endif