// Header file for Server class
#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

using std::vector;
using std::string;
class Server
{
    private:
        char *port;
        int listenfd;
    public:
        Server(char* port);
        char* get_port();
        bool start_server();
        void handle_client();
        int accept_client(char**);
        void recv_command(int newfd, char * [3]);
        void parse_command(char *, char * [3]);
        void list_directory(char *, char *);
        bool send_message(const char *, int&);
        vector<string> get_dir_contents();
        int open_data_connection(char *, char*);
        bool valid_filename(char *);
        bool is_directory(char *);
        bool transfer_file(int&, char *, char *, char *);
        char *get_file_contents(char*);
};


#endif
