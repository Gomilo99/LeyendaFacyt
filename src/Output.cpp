#include "../lib/Output.hpp"
#include <iostream>

void ConsoleOutput::print(const std::string& msg) {
    std::cout << msg;
}

void ConsoleOutput::printLine(const std::string& msg) {
    std::cout << msg << "\n";
}
