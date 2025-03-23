#include <iostream>
#include <string>
#include <fstream>
#include <ncurses.h>

#include "io/io_utils.h"
#include "util/utils.h"
#include "buffer/buffer.h"

int main(int argc, char* argv[]) {

    parse_init(argc, argv);

    initscr();              // Start curses mode
    noecho();
    keypad(stdscr, TRUE);

    create_buf(PATH);

    printw(BUF.c_str());

    int c;
    int x = 0;
    int y = 0;

    move(y, x);
    while ((c = getch()) != 'q') {
        std::cout << "val =" << c << std::endl;
        switch (c) {
            case BACKSPACE:
                delete_char_back(BUF);
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
                BUF.append({char(c)});
                break;
        }

        adjust_x(x, BUF, y);
        adjust_y(y, BUF);

        clear();
        printw(BUF.c_str());
        move(y, x);
        refresh();

    }

    endwin();
}