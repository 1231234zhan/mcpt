
#include <chrono>
#include <iostream>
#include <string>

#include "global.hpp"
#include "misc.hpp"

Timer::Timer()
{
}

void Timer::start()
{
    _now = std::chrono::steady_clock::now();
}

void Timer::end()
{
    _end = std::chrono::steady_clock::now();
}

void Timer::output(const std::string& s)
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _now);
    std::cout << s << " " << ms.count() << " ms" << std::endl;
}

void Timer::end_and_output(const std::string& s)
{
    _end = std::chrono::steady_clock::now();
    this->output(s);
}