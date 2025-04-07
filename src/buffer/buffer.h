#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <sstream>
#include <string>
#include <vector>

// holds contents of the file being worked on
class buffer {
  std::vector<std::string> data;

public:
  void initialize(std::string input) {
    std::stringstream ss{input};
    std::string tmp;
    while (getline(ss, tmp)) {
      data.emplace_back(tmp);
    }
  }

  std::string get_str_repr() const {
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

  void insert_at(int x, int y, char c) {
    std::string *str = &data[y];
    str->insert(x, 1, c);
  }

  void delete_at(int x, int y) {
    std::string *str = &data[y];
    str->erase(x, 1);
  }

  const std::string &get_line_at(int y) const { return data[y]; }

  int get_num_lines() const { return data.size(); }
};

extern buffer BUF;

void create_buf(const std::string &path);

void write_out_buf(const std::string &path, const std::string &buf);

#endif