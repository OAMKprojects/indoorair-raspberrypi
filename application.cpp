#include "application.hpp"

Application  *Application::instance;

Application::Application() 
{
    serial.reset(new Serial());
    instance = this;
    values.clear();
}

Application::~Application()
{
    
}

void Application::signalHandler(int param)
{
    std::cout << std::flush;
    std::cout << "Got SIGINT, Exiting" << std::endl;
    instance->running = false;
}

int Application::init(const std::string port_name)
{
    if (serial->openPort(port_name) != 0) return -1;
    std::cout << "Server inited succesfully" << std::endl;
    return 0;
}

void Application::parseData(const uint8_t &data)
{
    
}

int Application::start()
{
    running = true;
    signal(SIGINT, signalHandler);

    int read_bytes = 0;

    std::cout << "Server is starting" << std::endl;

    while(running) {

        read_bytes = serial->readPort();

        if (read_bytes < 0) {
            running = false;
            return -1;
        }
        else if (read_bytes) parseData(serial->getBufferAddress());
    }

    return 0;
}