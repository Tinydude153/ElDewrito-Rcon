#ifndef CONSOLE_UTIL_H
#define CONSOLE_UTIL_H

#include <windows.h>

namespace WindowsConsole {

class Console {

    public:
    int columns;
    int rows;
    void GetConsoleSize();
    Console& Size();

};

}

#endif