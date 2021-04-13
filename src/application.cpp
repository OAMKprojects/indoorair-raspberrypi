#include "application.hpp"

Application  *Application::instance;
bool          Application::searching;

Application::Application() 
{
    serial.reset(new Serial());
    instance = this;
    
    clearParser();

    #ifdef ADMIN_APP
    admin = false;
    #endif
}

Application::~Application()
{
    
}

#ifdef ADMIN_APP
void Application::setAdmin()
{
    admin = true;
    server.reset(new Server(PORT_NUMBER));
}

void Application::setValues(std::string &json_str)
{
    parser.temp_string.erase(std::remove(parser.temp_string.begin(), parser.temp_string.end(), '\n'), parser.temp_string.end());
    parser.temp_string.erase(std::remove(parser.temp_string.begin(), parser.temp_string.end(), '\r'), parser.temp_string.end());

    json_str += "{\"values\":{";// + parser.temp_string;
    std::string num_str;

    for (auto item = parser.values.begin(); item != parser.values.end(); item++) {
        num_str = std::to_string(item->second);
        json_str += "\"" + item->first + "\"" + ":" + num_str.substr(0, num_str.find(".") + 2) + ",";
    }

    auto time_now = std::chrono::steady_clock::now();
    int time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(time_now - time_start).count();

    std::string time_str;
    int tmp_t;

    if ((tmp_t = time_elapsed / 3600)) {
        if (!(tmp_t / 10)) time_str += "0";
        time_str += std::to_string(tmp_t) + ":";
    }
    else time_str += "00:";

    if ((tmp_t = (time_elapsed / 60) % 60)) {
        if (!(tmp_t / 10)) time_str += "0";
        time_str += std::to_string(tmp_t) + ":";
    }
    else time_str += "00:";

    tmp_t = time_elapsed % 60;
    if (!(tmp_t / 10)) time_str += "0";
    time_str += std::to_string(tmp_t);

    json_str += "\"Uptime\":\"" + time_str + "\"}}";
}
#endif

void Application::clearParser()
{
    parser.level = data_parser::none;
    parser.temp_name = "";
    parser.temp_value = "";
    parser.temp_string = "";
    parser.values.clear();
}

void Application::signalHandler(int param)
{
    std::cout << std::flush;
    std::cout << "Got SIGINT, Exiting" << std::endl;
    instance->running = false;

    #ifdef ADMIN_APP
    if (instance->admin) {
        instance->server->stop();
    }
    #endif
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
    parser.temp_string += data;

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
    //if (db_save) saveDataDB();

    for (auto item = parser.values.begin(); item != parser.values.end(); item++) {
        std::cout << item->first << " = " << item->second << "  ";
    }

    std::cout << std::endl;

    #ifdef ADMIN_APP
    if (admin) {
        std::string json_string;
        setValues(json_string);
        server->setMessage(json_string);
    }
    #endif

    clearParser();
}

int Application::start()
{
    running = true;
    signal(SIGINT, signalHandler);

    int read_bytes = 0;

    std::cout << "Server is starting" << std::endl;

    #ifdef ADMIN_APP
    if (admin) {
        time_start = std::chrono::steady_clock::now();
        thread_server = std::thread(&Server::start, server.get());
    }
    #endif

    while(running) {

        read_bytes = serial->readPort();

        if (read_bytes < 0) {
            running = false;
            return -1;
        }
        else if (read_bytes) {
            parseData(reinterpret_cast<const char *>(serial->getBufferAddress()));
        }
    }

    #ifdef ADMIN_APP
    if (admin) {
        thread_server.join();
    }
    #endif

    return 0;
}