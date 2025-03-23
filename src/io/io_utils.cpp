#include "io_utils.h"

#include <iostream>
#include <string>
#include <sstream>

std::string get_line_at(const std::string& buffer, int y) {
    std::istringstream stream(buffer);
    std::string line;
    for (int i = 0; i <= y && std::getline(stream, line); ++i);
    return line;
}

int count_lines(const std::string& buffer) {
    return std::count(buffer.begin(), buffer.end(), '\n');
}

void adjust_x(int &x, const std::string& buffer, const int y) {
    if (x < 0) x = 0;

    std::string current_line = get_line_at(buffer, y);
    x = std::min<int>(x, current_line.length());
}

void adjust_y(int &y, const std::string& buffer) {
    if (y < 0) y = 0;

    int num_lines = count_lines(buffer);
    y = std::min(y, num_lines);
}

void delete_char_back(std::string& input) {
    input.erase(input.size() - 1);
}

void print(const char * str) {
    std::cout << str << std::endl;
}