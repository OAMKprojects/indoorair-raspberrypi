#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <atomic>

#define BUFFER_SIZE     1024

#define ERR_CREATION    0
#define ERR_OPTIONS     1
#define ERR_RECEIVE     2
#define ERR_BIND        3
#define ERR_LISTEN      4
#define ERR_SOCKET      5
#define ERR_INTERFACE   7
#define ERR_CLIENT      8
#define ERR_READ        9
#define ERR_SEND        10

#define DEFAULT_KEY     "Indoor"
#define OPEN_PHRASE     "Enter Password"
#define WELCOME_PHRASE  "Welcome to Indoor Air server"
#define PASSWORD        "inadmin"

class Server
{
    public:
        Server(int _port);
        virtual ~Server();

        void start();
        void stop();
        void setMessage(const std::string &message);
        std::string getApplicationMessage();
        bool getAtomicFlag() {return application_message_set;}

    private:
        void freeResources();
        void raiseError(int type);
        void readClient(int cl);
        bool handleConnect();
        void setCanonicalMode(bool on_off);
        void openServerSocket();
        void initServerSocket();
        void serveAdmin();
        void handleMessage(int length);
        void setApplicationMessage();
        void setApplicationCommand(const std::string &command);

        std::string encodeMessage(const std::string &message);
        std::string decodeMessage(const std::string &message);

        struct Crypt {
            std::string key;
            int key_size;
            const int max_char = 126;
            const int min_char = 32;
        };

        sockaddr_in         admin_addr;
        int                 port_number;
        unsigned int        addr_size;
        int                 socket_admin, socket_server;
        int                 addres_in_len;
        char               *buffer;
        termios             t_old, t_new;
        int                 oldf;
        bool                canonical_mode;
        bool                admin_connected;
        std::atomic<bool>   admin_mesage_set;
        std::atomic<bool>   application_message_set;
        std::atomic<bool>   running;
        Crypt               crypt;
        std::string         my_ip;
        std::string         admin_message;
        std::string         application_message;
        std::mutex          mtx_ser, mtx_app;
};

#endif // SERVER_HPP
