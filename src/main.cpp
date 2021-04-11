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

    if (port_name == "debug") {
        app.debug();
        return 0;
    }

    if (app.init(port_name) < 0) return -1;

    #ifdef ADMIN_APP
    if (argv == 3) {
        if (strcmp(args[2], "admin") == 0) {
            app.setAdmin();
        }
    }
    #endif

    app.start();

    return 0;
}