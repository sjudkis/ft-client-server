
class Socketft
{
    private:
        char* port;
        char* host;
        int fd;
    public:
        Socketft(char * port, char *host);
        Socketft(char* host, int fd);
        bool start_listening();
        bool open_connection();
        Socketft accept_connection();
        char *recv_message();
        bool send_message(char* message);
};