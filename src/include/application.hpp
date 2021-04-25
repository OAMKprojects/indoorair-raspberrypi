#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <signal.h>
#include <algorithm>
#include <sqlite3.h>
#include <algorithm>
#include <chrono>
#include "serial.hpp"

#ifdef ADMIN_APP
#include <thread>
#include "server.hpp"

#define PORT_NUMBER    8080
#endif

#define DATABASE_FILE       "indoorair.db"
#define DEFAULT_SAVE_TIME   10
#define MAX_SAVE_TIME       99 * 3600 + 99 * 60 + 99

typedef std::map<std::string, std::string> StringMap;
typedef std::map<std::string, float> FloatMap;

class Application
{
    public:
        Application();
        ~Application();

        static void signalHandler(int param);
        static int  dbCallBack(void* data, int argc, char** argv, char** azColName);

        int init(const std::string port_name);
        int start();
        void debug();

        #ifdef ADMIN_APP
        void setAdmin();
        #endif

    private:

         struct data_parser {
            enum levels {
                none,
                begin_read_name,
                read_name,
                begin_read_value,
                read_value,
                read_string
            };
            int         level;
            char        ch;
            std::string temp_name;
            std::string temp_value;
            std::string temp_string;
            FloatMap    values;
            StringMap   strings;
        };

        bool openDatabase(const std::string db_name);
        void parseData(data_parser &parser, const char *data);
        void saveValue(data_parser &parser, bool more_data);
        void saveString(data_parser &parser, bool more_data);
        void endParsing(data_parser &parser);
        void saveDataDB();
        void clearParser(data_parser &parser);
        void printMessage(int verbose, const std::string &message, bool new_line = true);

        #ifdef ADMIN_APP
        void setValues(std::string &json_str);
        int addSeconds(const std::string &time_str, int time_level);
        std::string getTimeString(long seconds);
        void setTimeFromString(const std::string &time_str);
        void checkAdminCommands(data_parser &parser);
        #endif

        std::chrono::time_point<std::chrono::steady_clock> time_start;
        std::chrono::time_point<std::chrono::steady_clock> time_save;
        std::unique_ptr<Serial>  serial;
        static Application      *instance;
        static bool   searching;
        data_parser   main_parser;
        bool          running;
        bool          db_save;
        long          saving_time;
        int           verbose_level;
        sqlite3      *db;

        #ifdef ADMIN_APP
        std::unique_ptr<Server> server;
        std::thread             thread_server;
        data_parser             second_parser;
        StringMap               suffix_map;
        bool                    admin;
        bool                    control_update;
        std::string             com_string; 
        #endif
};

#endif