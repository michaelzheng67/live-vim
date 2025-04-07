#include <fstream>

#include "buffer.h"

buffer BUF;

void create_buf(const std::string &path) {
  std::fstream ifile(path, std::ios::binary | std::ios::in | std::ios::out);
  std::ostringstream ss;
  ss << ifile.rdbuf();
  BUF.initialize(ss.str());
  ifile.close();
}

void write_out_buf(const std::string &path, const std::string &buf) {
  std::ofstream ofile(path, std::ios::out | std::ios::trunc | std::ios::binary);
  ofile.write(buf.c_str(), buf.size());
  ofile.close();
}