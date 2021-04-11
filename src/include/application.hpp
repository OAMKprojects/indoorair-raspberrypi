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
#include "serial.hpp"

#ifdef ADMIN_APP
#include <thread>
#include "server.hpp"
#define PORT_NUMBER    8080
#endif

#define DATABASE_FILE   "indoorair.db"

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
        bool openDatabase(const std::string db_name);
        void parseData(const char *data);
        void saveValue(bool more_data);
        void saveDataDB();
        void clearParser();

        struct data_parser {
            enum levels {
                none,
                begin_read_name,
                read_name,
                begin_read_value,
                read_value
            };
            int         level;
            char        ch;
            std::string temp_name;
            std::string temp_value;
            std::string temp_string;
            std::map<std::string, float>  values;
        };

        std::unique_ptr<Serial>  serial;
        static Application      *instance;
        static bool   searching;
        data_parser   parser;
        bool          running;
        bool          db_save;
        sqlite3      *db;

        #ifdef ADMIN_APP
        std::unique_ptr<Server>  server;
        std::thread              thread_server;
        bool admin;
        #endif
};

#endif