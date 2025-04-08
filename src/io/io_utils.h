#ifndef __IO_UTILS_H_
#define __IO_UTILS_H_

#include "../buffer/buffer.h"

#include <fstream>

#define BACKSPACE 127 // int value of backspace key

/* --- terminal utils --- */

// call after each terminal input to ensure that cursor is at correct position
void adjust_x(int &x, const buffer &buf, const int y);
void adjust_y(int &y, const buffer &buffer);

// Given a string, we delete the last character of
// the file.
void delete_char_back(std::string &input);

// yes, im just that lazy
void print(const char *str);

#endif