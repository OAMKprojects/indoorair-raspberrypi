#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <signal.h>
#include "serial.hpp"

class Application
{
    public:
        Application();
        ~Application();

        static void signalHandler(int param);

        int init(const std::string port_name);
        int start();

    private:
        void parseData(const uint8_t &data);

        std::unique_ptr<Serial>  serial;
        static Application      *instance;
        std::map<std::string, float>  values;
        bool running;
};

#endif