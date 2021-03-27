#include <iostream>
#include <signal.h>
#include "serial.hpp"

void signalHandler(int param)
{
    std::cout << "Got SIGINT, Exiting" << std::endl;
    exit(0);
}

int main(int argv, char **args)
{
    if (argv == 1) {
        std::cout << "Enter serial port name" << std::endl;
        return 0;
    }

    std::string port_name = args[1];
    Serial serial;

    if (serial.openPort(port_name) != 0) return -1;

    signal(SIGINT, signalHandler);

    while(1) {
        if (serial.readPort() < 0) return -1;
    }

    return 0;
}