#include <iostream>
#include <string>
#include <fstream>
#include <ncurses.h>

#include "io/utils.h"

int main() {

    initscr();              // Start curses mode
    noecho();
    keypad(stdscr, TRUE);

    
    std::string tmp;
    std::fstream ifile("data/file.txt", std::ios::binary|std::ios::in|std::ios::out);
    
    std::string buffer((std::istreambuf_iterator<char>(ifile)),
                       std::istreambuf_iterator<char>());

    printw(buffer.c_str());

    int c;
    int x = 0;
    int y = 0;

    move(y, x);
    while ((c = getch()) != 'q') {
        std::cout << "val =" << c << std::endl;
        switch (c) {
            case BACKSPACE:
                delete_char_back(buffer);
                delch();
                break;
            case KEY_LEFT:
                x--;
                break;
            case KEY_RIGHT:
                x++;
                break;
            case KEY_UP:
                y--;
                break;
            case KEY_DOWN:
                y++;
                break;
            default:
                buffer.append({char(c)});
                break;
        }

        adjust_x(x, buffer, y);
        adjust_y(y, buffer);

        clear();
        printw(buffer.c_str());
        move(y, x);
        refresh();

    }

    endwin();

    ifile.close(); // close input file

    // write out buffer
    std::ofstream ofile("data/file.txt", std::ios::out | std::ios::trunc | std::ios::binary);
    ofile.write(buffer.c_str(), buffer.size());
    ofile.close();
}