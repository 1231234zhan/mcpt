#pragma once

#include <chrono>
#include <string>

#include "global.hpp"

class Timer {
private:
    std::chrono::time_point<std::chrono::steady_clock> _now, _end;

public:
    Timer();
    void start();
    void end();
    void output(const std::string&);
    void end_and_output(const std::string&);
};