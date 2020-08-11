#include "lexer.hpp"

int main() {
    bool b = true;
    auto a = lex(getLines("file.txt"), "file.txt", b);

    for (auto it = a.begin(); it < a.end(); it++) std::cout << (*it).lexeme << " -> " << (*it).type << " | ";

    std::cout << "\nEND OF PROGRAM";
}