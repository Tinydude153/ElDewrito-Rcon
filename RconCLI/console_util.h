#ifndef CONSOLE_UTIL_H
#define CONSOLE_UTIL_H

#if _WIN32 
    #include <windows.h>
#else 
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

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