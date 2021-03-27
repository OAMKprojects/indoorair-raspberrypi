#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <error.h>

#define BAUD_RATE       B9600
#define BUFFER_SIZE     256

#define ERR_OPEN_PORT   1
#define ERR_GET_TCG     2
#define ERR_SET_TCG     3
#define ERR_READ        4

class Serial
{
    public:
        Serial();
        ~Serial();

        int openPort(std::string port_name);
        int readPort();

    private:
        void printError(const int err_num);

        int port_num;
        int error_num;
        uint8_t buffer[BUFFER_SIZE];
};

#endif