#include "server.hpp"

Server::Server(int _port)
{
    port_number = _port;
    canonical_mode = false;

    buffer = new char[BUFFER_SIZE];
    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);

    initServerSocket();

    ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "wlp2s0", IFNAMSIZ-1);
    ioctl(socket_server, SIOCGIFADDR, &ifr);

    my_ip = inet_ntoa(((sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    crypt.key = DEFAULT_KEY;
    crypt.key_size = crypt.key.length();
    admin_connected = false;
}

Server::~Server()
{
    freeResources();
}

void Server::freeResources()
{
    if (buffer) delete[] buffer;
    setCanonicalMode(false);

    close(socket_server);
    close(socket_admin);
}

void Server::raiseError(int type)
{
    switch(type)
    {
        case ERR_CREATION: perror("Error when creating socket"); break;
        case ERR_OPTIONS: perror("Error when changing socket options"); break;
        case ERR_RECEIVE: perror("Error when receiving packets"); break;
        case ERR_BIND: perror("Error when binding socket"); break;
        case ERR_LISTEN: perror("Error when listening socket"); break;
        case ERR_SOCKET: perror("Error when polling sockets"); break;
        case ERR_INTERFACE: perror("Error when setting interface"); break;
        case ERR_CLIENT: perror("Error when adding client"); break;
        case ERR_READ: perror("Error when reading message"); return; break;
        case ERR_SEND: perror("Error when sending message"); return; break;
    }

    freeResources();
    exit(EXIT_FAILURE);
}

void Server::setCanonicalMode(bool on_off)
{

    if (on_off && !canonical_mode)
    {
        tcgetattr(fileno(stdin), &t_old);
        memcpy(&t_new, &t_old, sizeof(termios));
        t_new.c_lflag &= ~(ECHO | ICANON);
        t_new.c_cc[VTIME] = 0;
        t_new.c_cc[VMIN] = 1;
        tcsetattr(fileno(stdin), TCSANOW, &t_new);

        oldf = fcntl(fileno(stdin), F_GETFL, 0);
        fcntl(fileno(stdin), F_SETFL, oldf | O_NONBLOCK);
        canonical_mode = true;
    }
    else if (!on_off && canonical_mode)
    {
        tcsetattr(fileno(stdin), TCSANOW, &t_old);
        fcntl(fileno(stdin), F_SETFL, oldf);
        canonical_mode = false;
    }

}

void Server::initServerSocket()
{
    int opt = 1;

    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if ((socket_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) raiseError(ERR_CREATION);
    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) raiseError(ERR_OPTIONS);
    if (setsockopt(socket_server, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) raiseError(ERR_OPTIONS);

    bzero(&admin_addr, sizeof(admin_addr));

    admin_addr.sin_family = AF_INET;
    admin_addr.sin_addr.s_addr = INADDR_ANY;
    admin_addr.sin_port = htons(port_number);
}

void Server::openServerSocket()
{
    if (bind(socket_server, (sockaddr *)&admin_addr, addr_size) < 0) raiseError(ERR_BIND);
    if (listen(socket_server, 3) < 0) raiseError(ERR_LISTEN);
}

std::string Server::encodeMessage(const std::string &message)
{
    std::string result;
    int counter = 0;
    int new_char;
    
    for (auto &ch : message) {
        new_char = (int)ch + (int)crypt.key[counter++ % crypt.key_size];
        if (new_char > crypt.max_char) {
            new_char = crypt.min_char + new_char - crypt.max_char;
            if (new_char > crypt.max_char) new_char = crypt.min_char + new_char - crypt.max_char;
        }
        result += (char)new_char;
    }

    return result;
}

std::string Server::decodeMessage(const std::string &message)
{
    std::string result;
    int counter = 0;
    int new_char;
    
    for (auto &ch : message) {
        new_char = (int)ch - (int)crypt.key[counter++ % crypt.key_size];
        if (new_char < 0) {
            new_char = crypt.max_char - (abs(new_char) + crypt.min_char);
            if (new_char < 0) new_char = crypt.max_char - (abs(new_char) + crypt.min_char);
            else if (new_char < crypt.min_char) new_char = crypt.max_char - (crypt.min_char - new_char);
        }
        else if (new_char < crypt.min_char) new_char = crypt.max_char - (crypt.min_char - new_char);
        result += (char)new_char;
    }

    return result;
}

bool Server::handleConnect()
{
    if (send(socket_admin, encodeMessage(OPEN_PHRASE).c_str(), strlen(OPEN_PHRASE), 0) < 0) {
        raiseError(ERR_SEND);
        return false;
    }
    
    timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(socket_admin, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int recv_bytes = read(socket_admin, buffer, BUFFER_SIZE - 1);
    if (recv_bytes < 0) {
        if (errno == EAGAIN) std::cout << "Client didn't response" << std::endl;
            else raiseError(ERR_READ);

        close(socket_admin);
        return false;
    }
    else if (recv_bytes == 0) {
        std::cout << "Client disconnected" << std::endl;

        close(socket_admin);
        return false;
    }

    if (strcmp(decodeMessage(buffer).c_str(), PASSWORD) != 0) {
        send(socket_admin, encodeMessage("Invalid Password!").c_str(), strlen("Invalid Password!"), 0);
        std::cout << "Invalid Password! : " << decodeMessage(buffer).c_str() << std::endl;

        close(socket_admin);
        return false;
    }

    tv.tv_sec = 1;
    setsockopt(socket_admin, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    send(socket_admin, encodeMessage(WELCOME_PHRASE).c_str(), strlen(WELCOME_PHRASE), 0);
    std::cout << "Correct password, admin connected" << std::endl;

    return true;
}

void Server::serveAdmin()
{
    if (admin_mesage_set) {

        //admin_message = "{\"temperature\":" + std::to_string(rand() % 20) + ",\"humidity\":20}";
        int bytes_send = send(socket_admin, encodeMessage(admin_message).c_str(), admin_message.length(), 0);

        admin_mesage_set = false;

        if (bytes_send < 0) {
            raiseError(ERR_SEND);

            close(socket_admin);
            admin_connected = false;
            return;
        }

        std::cout << admin_message << std::endl;
    }

    int recv_bytes = read(socket_admin, buffer, BUFFER_SIZE - 1);
    if (recv_bytes == 0) {
        std::cout << "Admin disconnected!" << std::endl;

        close(socket_admin);
        admin_connected = false;
        return;
    }
    else if (recv_bytes < 0) {
        if (errno != EAGAIN && errno != EINTR) raiseError(ERR_SOCKET);
    }
}

void Server::setMessage(const std::string &message)
{
    if (admin_mesage_set) return;

    std::lock_guard<std::mutex> lock(mtx);
    admin_message = message;
    admin_mesage_set = true;
}

void Server::stop()
{
    std::lock_guard<std::mutex> lock(mtx);
    running = false;
}

void Server::start()
{
    addr_size = sizeof(admin_addr);
    admin_mesage_set = false;
    int data_size, activity;
    int sd, max_sd;

    openServerSocket();

    setCanonicalMode(true);
    running = true;

    std::cout << "Server start: IP = " << my_ip << " PORT = "<< port_number << std::endl;

    while(running) {
        
        if (!admin_connected) {
            if ((socket_admin = accept(socket_server, (sockaddr *)&admin_addr, (socklen_t *)&addr_size)) < 0) {
                if (errno != EAGAIN && errno != EINTR) raiseError(ERR_SOCKET);
            }
            else admin_connected = handleConnect();
        }
        else serveAdmin();
    }
}
