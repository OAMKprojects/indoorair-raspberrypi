#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <signal.h>
#include <algorithm>
#include <sqlite3.h>
#include "serial.hpp"

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
            std::map<std::string, float>  values;
        };

        std::unique_ptr<Serial>  serial;
        static Application      *instance;
        data_parser   parser;
        bool          running;
        bool          db_save;
        sqlite3      *db;
};

#endif