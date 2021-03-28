#include <iostream>
#include "application.hpp"

int main(int argv, char **args)
{
    if (argv == 1) {
        std::cout << "Enter serial port name" << std::endl;
        return 0;
    }

    std::string port_name = args[1];
    Application app;

    if (app.init(port_name) < 0) return -1;

    app.start();

    return 0;
}