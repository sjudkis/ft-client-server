// Header file for Server class
#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include "Socketft.hpp"

using std::vector;
using std::string;
class Server
{
    private:
        char *port;
        Socketft *listen_socket;
    public:
        Server(char* port);
        char* get_port();
        bool start_server(); //
        void handle_client(Socketft*);
        Socketft *accept_client();
        void recv_command(Socketft *, char * [3]);
        void parse_command(char *, char * [3]);
        void list_directory(char *, char *);
        // bool send_message(const char *, int&);
        vector<string> get_dir_contents();
        // int open_data_connection(char *, char*); //
        bool valid_filename(char *);
        bool is_directory(char *);
        bool transfer_file(Socketft *, char *, char *);
        char *get_file_contents(char*);
};


#endif
