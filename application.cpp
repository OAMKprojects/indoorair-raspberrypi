#include "application.hpp"

Application  *Application::instance;

Application::Application() 
{
    serial.reset(new Serial());
    instance = this;
    
    clearParser();
}

Application::~Application()
{
    
}

void Application::clearParser()
{
    parser.level = data_parser::none;
    parser.temp_name = "";
    parser.temp_value = "";
    parser.values.clear();
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

void Application::parseData(const char *data)
{
    int i = 0;
    while((parser.ch = data[i++]) != '\0') {
        switch(parser.level) {
            case data_parser::none:
                if (parser.ch == '{') parser.level = data_parser::begin_read_name;
                break;
            case data_parser::begin_read_name:
                if (parser.ch == '"') parser.level = data_parser::read_name;
                break;
            case data_parser::read_name:
                if (parser.ch == '"') parser.level = data_parser::begin_read_value;
                else parser.temp_name += parser.ch;
                break;
            case data_parser::begin_read_value:
                if (parser.ch == ':') parser.level = data_parser::read_value;
                break;
            case data_parser::read_value:
                if (parser.ch == ',') saveValue(true);
                else if (parser.ch == '}') saveValue(false);
                else parser.temp_value += parser.ch;
                break;
        }
    }
}

void Application::saveValue(bool more_data)
{
    parser.temp_value.erase(std::remove(parser.temp_value.begin(), parser.temp_value.end(), ' '), parser.temp_value.end());
    if (more_data) parser.level = data_parser::begin_read_name;
    else parser.level = data_parser::none;

    float num;
    try {
        num = (float)std::stod(parser.temp_value);
    }
    catch (...){
        clearParser();
        return;
    }

    parser.values.insert(std::pair<std::string, float>(parser.temp_name, num));
    if (more_data) return;

    for (auto item = parser.values.begin(); item != parser.values.end(); item++) {
        std::cout << item->first << " = " << item->second << std::endl;
    }
    clearParser();
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
        else if (read_bytes) parseData((char *)serial->getBufferAddress());
    }

    return 0;
}