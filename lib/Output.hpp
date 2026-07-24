#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <string>

class Output {
    public:
        virtual ~Output() = default;
        virtual void print(const std::string& msg) = 0;
        virtual void printLine(const std::string &msg) = 0;
};

class ConsoleOutput : public Output{
    public:
        void print(const std::string &msg) override;
        void printLine(const std::string &msg) override;
};

class NullOutput : public Output {
    public:
        void print(const std::string& /*msg*/) override {}
        void printLine(const std::string& /*msg*/) override {}
};

#endif