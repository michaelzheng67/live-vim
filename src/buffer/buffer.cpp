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

void buffer::initialize(std::string input) {
  std::stringstream ss{input};
  std::string tmp;
  while (getline(ss, tmp)) {
    data.emplace_back(tmp);
  }
}

std::string buffer::get_str_repr() const {
  std::string res;
  unsigned int n = data.size();
  for (int i = 0; i < n; i++) {
    res += data[i];
    if (i < n - 1) {
      res += '\n';
    }
  }
  return res;
}

std::tuple<int, int> buffer::insert_at(int x, int y, char c) {
  std::string *str = &data[y];

  // split current line into two
  if (c == '\n') {
    std::string first = str->substr(0, x);
    std::string second = str->substr(x);
    *str = first;
    data.insert(data.begin() + y + 1, second);
    y++;
    x = 0;
  } else {
    str->insert(x, 1, c);
    x++;
  }
  data.shrink_to_fit();
  return std::tuple<int, int>{x, y};
}

std::tuple<int, int> buffer::delete_at(int x, int y) {

  x--;
  std::string *str = &data[y];

  if (x <= 0) {

    // shift over lines if the current one is to be deleted
    if (y > 0) {

      x = data[y - 1].length();
      data[y - 1] = data[y - 1] + data[y];

      for (int i = y; i < data.size() - 1; i++) {
        data[i] = data[i + 1];
      }

      data.pop_back();
      data.shrink_to_fit();
      y--;
    }
  } else if (str->size() > 0) {
    str->erase(x, 1);
  }
  return std::tuple<int, int>{x, y};
}

const std::string &buffer::get_line_at(int y) const { return data[y]; }

int buffer::get_num_lines() const { return data.size(); }
