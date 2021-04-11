#include "serial.hpp"

Serial::Serial() 
{
    memset(buffer, 0, sizeof(buffer));
}

Serial::~Serial() {}

void Serial::printError(const int err_num)
{
    error_num = err_num;
    switch (error_num) {
        case _ERR_OPEN_PORT: perror("Error when opening serial port"); break;
        case _ERR_GET_TCG: perror("Error when getting address"); break;
        case _ERR_SET_TCG: perror("Error when setting address"); break;
        case _ERR_READ: perror("Error when reading serial port"); break;
    }
}

int Serial::openPort(std::string port_name)
{
    std::string port_file = "/dev/" + port_name;
    
    if ((port_num = open(port_file.c_str(), O_RDONLY)) < 0) {
        printError(_ERR_OPEN_PORT);
        return -error_num;
    }

    struct termios tty;

    if(tcgetattr(port_num, &tty) != 0) {
        printError(_ERR_GET_TCG);
        return -error_num;
    }

    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CREAD | CLOCAL);
    tty.c_cflag |= CS8 | CREAD | CLOCAL;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG | IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT| PARMRK| ISTRIP| INLCR | IGNCR | ICRNL | OPOST | ONLCR);

    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;

    cfsetspeed(&tty, BAUD_RATE);

    if (tcsetattr(port_num, TCSANOW, &tty) != 0) {
        printError(_ERR_SET_TCG);
        return -error_num;
    }

    return 0;
}

int Serial::readPort()
{
    int num_bytes = read(port_num, &buffer, _BUFFER_SIZE - 1);

    if (num_bytes < 0) {
        printError(_ERR_READ);
        return -error_num;
    }
    else if (num_bytes) {
        buffer[num_bytes] = '\0';
        //std::cout << buffer;
    }

    return num_bytes;
}