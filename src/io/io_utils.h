#ifndef __IO_UTILS_H_
#define __IO_UTILS_H_

#include <fstream>

#define BACKSPACE 127 // int value of backspace key

/* --- terminal utils --- */

// gets the line number that the cursor is currently at
std::string get_line_at(const std::string& buffer, int y);

// count number of lines in text file
int count_lines(const std::string& buffer);

// call after each terminal input to ensure that cursor is at correct position
void adjust_x(int &x, const std::string& buffer, const int y);
void adjust_y(int &y, const std::string& buffer);



// Given a fstream, we delete the last character of
// the file.
void delete_char_back(std::string& input);

// yes, im just that lazy
void print(const char * str);

#endif