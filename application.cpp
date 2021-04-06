#include "application.hpp"

Application  *Application::instance;
bool          Application::searching;

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

int Application::dbCallBack(void* data, int argc, char** argv, char** azColName)
{
    int i;
    //fprintf(stderr, "%s: ", (const char*)data);
  
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
  
    printf("\n");
    searching = false;
    return 0;
}

void Application::debug()
{
    int err = sqlite3_open(DATABASE_FILE, &db);
    if (err != SQLITE_OK) {
        std::cout << "Error when opening database: " << sqlite3_errstr(err) << std::endl;
        return;
    }

    const char *query = "SELECT DATETIME(time, 'localtime') as time, temperature, humidity FROM indoorair;";
    searching = true;
    sqlite3_exec(db, query, dbCallBack, NULL, NULL);
    while(searching);
}

bool Application::openDatabase(const std::string db_name)
{
    int err = sqlite3_open(db_name.c_str(), &db);
    if (err != SQLITE_OK) {
        std::cout << "Error when opening database: " << sqlite3_errstr(err) << std::endl;
        return false;
    }

    const char *query = "CREATE TABLE IF NOT EXISTS indoorair"
                        "(id INTEGER PRIMARY KEY AUTOINCREMENT"
                        ", temperature DECIMAL(3, 1)"
                        ", humidity DECIMAL(3, 1)"
                        ", time ts TIMESTAMP DEFAULT 'localtime');";
    sqlite3_exec(db, query, dbCallBack, NULL, NULL);

    std::cout << "Database opened succesfully" << std::endl;
    return true;
}

void Application::saveDataDB()
{
    char query[256];
    sprintf(query, "INSERT INTO indoorair(temperature, humidity) VALUES('%.1f', '%.1f');",
            parser.values["temperature"], parser.values["humidity"]);
    sqlite3_exec(db, query, dbCallBack, NULL, NULL);
}

int Application::init(const std::string port_name)
{
    if (serial->openPort(port_name) != 0) return -1;
    db_save = openDatabase(DATABASE_FILE);

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
        parser.temp_name = "";
        parser.temp_value = "";
        return;
    }

    parser.values.insert(std::pair<std::string, float>(parser.temp_name, num));
    parser.temp_name = "";
    parser.temp_value = "";
    if (more_data) return;
    if (db_save) saveDataDB();

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