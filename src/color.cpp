#include <windows.h>
#include <wincon.h>
#include <string>
#include <iostream>

void COLOR(std::string text, int num) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, num);
    std::cout << text;
    std::cout.flush();
    SetConsoleTextAttribute(hConsole, 7);
    return;
#endif
    std::cout << text;
    std::cout.flush();
}