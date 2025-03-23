#include <iostream>

#include "utils.h"

void parse_init(int argc, char* argv[]) {
    switch (argc) {

        case 2: {

            break;
        }

        case 1: {
            PATH = argv[1];
            break;
        }
        default: {
            break;
        }
    }

}