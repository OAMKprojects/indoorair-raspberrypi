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

#define BAUD_RATE        B9600
#define _BUFFER_SIZE     256

#define _ERR_OPEN_PORT   1
#define _ERR_GET_TCG     2
#define _ERR_SET_TCG     3
#define _ERR_READ        4
#define _ERR_WRITE       5

class Serial
{
    public:
        Serial();
        ~Serial();

        int openPort(std::string port_name);
        int readPort();
        int writePort(const std::string &com_chars);
        uint8_t* getBufferAddress() {return buffer;}

    private:
        void printError(const int err_num);

        int port_num;
        int error_num;
        uint8_t buffer[_BUFFER_SIZE];
};

#endif