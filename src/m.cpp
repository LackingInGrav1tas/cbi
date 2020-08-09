#include "lexer.hpp"

int main() {
    bool b = true;
    std::vector<std::string> lines = { "abc", "num 2 #@*^(*", "3333 'abc!'" };
    auto a = lex(lines, "file!", b);
    for (auto it = a.begin(); it < a.end(); it++) {
        std::cout << (*it).lexeme << "|";
    }
}