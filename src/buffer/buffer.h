#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <sstream>
#include <string>
#include <tuple>
#include <vector>

// holds contents of the file being worked on
class buffer {
  std::vector<std::string> data;

public:
  void initialize(std::string input);

  std::string get_str_repr() const;

  std::tuple<int, int> insert_at(int x, int y, char c);

  std::tuple<int, int> delete_at(int x, int y);

  const std::string &get_line_at(int y) const;

  int get_num_lines() const;
};

extern buffer BUF;

void create_buf(const std::string &path);

void write_out_buf(const std::string &path, const std::string &buf);

#endif