
#include <random>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION

#include "global.hpp"
#include "misc.hpp"
#include "scene.hpp"

// FOR SEGMENTATION FAULT DEBUG
/******************************************/
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig)
{
    void* array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
/******************************************/

std::random_device dev;
std::mt19937 mt(dev());
std::uniform_real_distribution<flt> rng { 0, 1 };

int main(int argc, char** argv)
{
    signal(SIGSEGV, handler);
    std::string inputname = "input";
    int sample_num = 30;
    if (argc > 1) {
        inputname = std::string(argv[1]);
    }
    if (argc > 2) {
        sample_num = std::stoi(argv[2]);
    }

    Scene scene(inputname);
    Timer timer;

    timer.start();
    scene.render(inputname + ".jpg", sample_num);
    timer.end();

    timer.end_and_output("Render elasped time:");
    return 0;
}
