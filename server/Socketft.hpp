// Header file for Server class
#ifndef SOCKETFT_HPP
#define SOCKETFT_HPP

class Socketft
{
    private:
        char* port;
        char* host;
        int fd;
    public:
        Socketft(char *port); // for listening socket
        Socketft(char * port, char *host); // for remote connection socket
        Socketft(char* host, int fd); // for newly accepted socket
        char *getHost();
        char *getPort();
        bool start_listening();
        bool open_connection();
        Socketft *accept_connection();
        char *recv_message();
        bool send_message(const char* message);
        void close_socket();

        
};

#endif