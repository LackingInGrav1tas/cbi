#ifdef _WIN32
#include <windows.h>
#include <wincon.h>
#endif
#include <string>
#include <iostream>

#include "color.hpp"

void COLOR(std::string text, int num) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, num);
    if (num != DISPLAY_RED) {
        std::cout << text;
        std::cout.flush();
    } else {
        std::cerr << text;
        std::cerr.flush();
    }
    SetConsoleTextAttribute(hConsole, DISPLAY_WHITE);
    return;
#endif
    std::cout << text;
    std::cout.flush();
}