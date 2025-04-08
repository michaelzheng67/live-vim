#include "io_utils.h"

#include <iostream>
#include <sstream>
#include <string>

void adjust_x(int &x, const buffer &buf, const int y) {
  if (x < 0)
    x = 0;

  const std::string &current_line = buf.get_line_at(y);
  x = std::min<int>(x, current_line.length());
}

void adjust_y(int &y, const buffer &buf) {
  if (y < 0)
    y = 0;

  int num_lines = buf.get_num_lines();
  y = std::min(y, num_lines - 1);
}

void delete_char_back(std::string &input) { input.erase(input.size() - 1); }

void print(const char *str) { std::cout << str << std::endl; }