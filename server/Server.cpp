#include "Server.hpp"
#include "Socketft.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <vector>
#include <sys/ioctl.h>
#include <sys/stat.h>

using std::string;
using std::cout;
using std::endl;
using std::vector;

// constructor
// accepts string containing port number passed in as argument
// assigns to port member variable
Server::Server(char *p)
{
    port = p;
    listen_socket = new Socketft(p);
}

// getter function for server port number
char* Server::get_port()
{
    return port;
}

/**************************************************
 * member function to create socket, bind to port, and start listening
 * Inputs: 
 *      - No params, uses member variables containing port number and socket
 * Outputs: 
 *      - returns true if server started successfully, else false
 *      - if successful, member variable listenfd is an actively 
 *        listening socket
 * based on Beej's Guide
**************************************************/
 bool Server::start_server()
{
    if (listen_socket->start_listening())
        return true;
    return false;
}



/**************************************************
 * function to receive command sent by client. ensures entire command is 
 * received. creates single string containing all commands, then calls 
 * another member function to parse this string, copying each arg into 
 * the appropriate array element. These strings have been dynamically 
 * allocated and need to be deleted elsewhere
 * Inputs:
 *      - int, an actively connected socket
 *      - char * array, as buffer to hold commands, each element is NULL
 * Outputs:
 *      - no return value
 *      - if successful, char * array contains commands send by client
 *      - if not successful, each element is still NULL
**************************************************/
void Server::recv_command(Socketft *client, char *command_array[3])
{
    char *command_string = client->recv_message();

    // complete command received, parse it into individual args
    parse_command(command_string, command_array);

    // delete mem allocated for command string
    delete(command_string);
}


/**************************************************
 * function to accept a client connection from the listening
 * socket. copies the name of the connecting host to a passed
 *  in buffer.
 * Inputs: 
 *      - char **, address of char * to store name of connecting client.
 * Outputs:
 *      - returns an actively connected socket fd
 *        This is dynamicall allocated and needs to be deleted elsewhere
 * getnameinfo() functionality based on Beej's guide
**************************************************/
Socketft *Server::accept_client()
{
    return listen_socket->accept_connection();
}



/**************************************************
 * Parses single string containing all client command args. stores args in
 * char * array.
 * Inputs:
 *      - char *, contains string of all commands from client
 *      - char * array, each char * in array is NULL, use as storage for commands
 * Outputs:
 *      - no return value
 *      - command array will contain pointers to dynammically allocated strings
 *        for each arg
 * strtok portion based off example found here:
 * https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
**************************************************/
void Server::parse_command(char *command, char *command_array[3])
{
    const char delim[2] = " ";
    char * arg = strtok(command, delim);
    
    int num_args = 0;
    while (arg != NULL && num_args < 3)
    {
        command_array[num_args] = new char[strlen(arg) + 1];
        strcpy(command_array[num_args], arg);
        num_args++;
        arg = strtok(NULL, delim);

    }



}

/**************************************************
 * function to control the flow of an entire client connection,
 * using other member functions to start connection, get commands, 
 * and execute commands. 
 * Inputs:
 *      - none
 * Outputs:
 *      - none. when complete, the entire client connection as ended.
 *      - prints information about the connection process as it goes on
 *        including any errors
**************************************************/
void Server::handle_client(Socketft *client)
{

    // print name of client host
    cout << "Connection from " << client->getHost() << endl;
    
    /************************************************/
    // get command from client
    /************************************************/
    // call function to receive command from client, passing in socket
    // and array to contain the parsed args. if there was an error or 
    // command was invalid, command_array[0] will remain NULL.
    char *command_array[3];
    for (int i = 0; i < 3; i++)
        command_array[i] = NULL;

    recv_command(client, command_array);
    if (command_array[0] == NULL)
    {
        // unable to receive command from socket
        return;
    }


    /************************************************/
    // execute command
    /************************************************/

    char * command = command_array[0];

    // received list directory command
    if (strcmp(command, "-l") == 0)
    {
        char *data_port = command_array[1];

        // send OK status message on command socket
        if (client->send_message("OK"))
            list_directory(data_port, client->getHost());   // handle -l command
        else
            fprintf(stderr,"ERROR: unable to send status message to client command socket\n");
    }

    // received get file command
    else if (strcmp(command, "-g") == 0)
    {
        char * filename = command_array[1];
        char * data_port = command_array[2];


        printf("File \"%s\" requested on port %s\n", filename, data_port);

        transfer_file(client, data_port, filename);

    }

    // delete contents of command_array, close command socket
    for (int i = 0; i < 3; i++)
    {
        if (command_array[i] != NULL)
            delete command_array[i];
    }
    
    client->close_socket();
}

/**************************************************
 * Sends a message over a socket connection.
 * format of message: <message_length>$<message_text>
 * prepends message with length of the message portion, so receiver 
 * knows how much data to expect. length and text separated by "$"
 * Inputs:
 *      - const char *, contains message to be sent
 *      - int&, an actively connected socket fd
 * Outputs:
 *      - bool, true if message sent, false if it failed
**************************************************/
// bool Server::send_message(const char *message, int &fd)
// {
//     int msg_len = strlen(message);  // length of message portion
//     char len_buf[20];               // hold string containing msg_len chars
//     memset(len_buf, '\0', 20);
    
//     // convert msg_len into string stored in len_buf
//     sprintf(len_buf, "%d", msg_len);

//     int total_len = strlen(len_buf) + msg_len + 1;
//     char *complete_message = new char[total_len + 1];
//     strcpy(complete_message, len_buf);
//     strcat(complete_message, "$");
//     strcat(complete_message, message);

//     // attempt to send message to socket, check for error
//     int total_sent = send(fd, complete_message, total_len, 0);
//     if (total_sent <= 0)
//         return false;

//     // make sure entire message was sent
//     int check_send = -19;
//     do 
//     {
//         ioctl(fd, TIOCOUTQ, &check_send); 
//     }
//     while (check_send > 0);
//     if (check_send < 0) 
//         return false;

//     // message sent successfully
//     delete complete_message;
//     return true;
// }


/**************************************************
 * function to return a string vector containing the names of all
 * non-hidden files in current directory.
 * Inputs:
 *      - none
 * Outputs:
 *      - vector of filename strings
 * files. uses the dirent struct described here:
 * https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html
**************************************************/
vector<string> Server::get_dir_contents()
{
    // string vector to contain filenames
    vector<string> contents;
    DIR *current_dir = opendir(".");

    struct dirent *file = readdir(current_dir);
    while (file != NULL)
    {
        // get name of each file in dir, if not hidden
        if (file->d_name[0] != '.')
            contents.push_back(string(file->d_name));
        
        file = readdir(current_dir);
    }
    closedir(current_dir);

    return contents;
}

// function to client's data transfer socket.
// accepts 2 char * parameters containing port and host to connect to
/**************************************************
 * function to open data transfer connection with client. uses port number 
 * specified by client in command.
 * Inputs:
 *      - char *,  port number to use
 *      - char *, host name to connect to
 * Outputs:
 *      - int, either an actively connected socket fd, or -1 if error 
**************************************************/
// int Server::open_data_connection(char * data_port, char *host)
// {
    
//     // create structs for address info
//     struct addrinfo hints, *res;

//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_UNSPEC;        // either IPv4 or IPv6
//     hints.ai_socktype = SOCK_STREAM;    // use TCP socket
//     hints.ai_flags = AI_PASSIVE;

//     // get address info for destination host
//     getaddrinfo(host, data_port, &hints, &res);
    

//     // create socket
//     int datafd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//     if (datafd < 0)
//     {    
//         fprintf(stderr, "ERROR: unable to create data socket\n");
//         fflush(stderr);
//         freeaddrinfo(res);
//         return -1;
//     }

    
//     // connect socket to server
//     if (connect(datafd, res->ai_addr, res->ai_addrlen) < 0)
//     {
//         fprintf(stderr, "ERROR: unable to start data transfer connection with host %s:%s\n", 
//                 host, data_port);
//         fflush(stderr);
//         freeaddrinfo(res);
//         return -1;
//     }

    
//     freeaddrinfo(res);

//     // successfully connected to client's data transfer socket
//     return datafd;
// }

/**************************************************
 * Function to handle a list directory command. calls member functions 
 * to get dir contents, open data transfer connection, and send content
 * listing
 * Inputs:
 *      - char *, data port to connect to 
 *      - char *, connected host's name
 * Outputs:
 *      - no returns, when complete, command has beenb fulfilled
 *        or error message is printed
**************************************************/
void Server::list_directory(char *data_port, char *connected_host)
{
    printf("List directory requested on port %s\n", data_port);

    // get all non-hidden filenames
    vector<string> dir_contents = get_dir_contents();
    
    int num_files = dir_contents.size();
    
    // build string of all filenames
    string content_string = "";
    for (int i = 0; i < num_files; i++)
    {
        // add filename to string, separated by newline
        content_string += dir_contents[i];
        if (i != num_files - 1)
            content_string += '\n';
    }
    
    // open connection to client on data port, send content string
    Socketft data_socket(data_port, connected_host);

    // check connection was successful
    if (data_socket.open_connection())
    {
        // send contents of directory, checking for error
        printf("Sending directory contents to %s:%s\n", connected_host, data_port);
        fflush(stdout);
        if (!data_socket.send_message(content_string.c_str()))
        {
            fprintf(stderr, "ERROR: unable to send directory contents to %s:%s\n", 
                    connected_host, data_port);
            fflush(stderr);
        }
        data_socket.close_socket();
    }
}


/**************************************************
 * checks if a given filename exists in current directory. gets list of 
 * all existing files, compares with given name. doesn't allow for 
 * path to file
 * Inputs:
 *      - char *, filename to check
 * Outputs:
 *      - bool, true if file exists, false if not
**************************************************/
bool Server::valid_filename(char *filename)
{
    // get names of all files in directory
    vector<string> dir_contents = get_dir_contents();
    
    int num_files = dir_contents.size();
    // check each file in directory for a match
    for (int i = 0; i < num_files; i++)
    {
        if (string(filename) == dir_contents[i])
            return true;    // match found
    }
    // no match found
    return false;
}


/**************************************************
 * tells if a given filename is a directory
 * Inputs:
 *      - char *, filename to check
 * Outputs:
 *      - bool, true if is directory, false if not
 * uses stat function code found here:
 * http://forum.codecall.net/topic/68935-how-to-test-if-file-or-directory/
**************************************************/
bool Server::is_directory(char *filename)
{
    struct stat stat_buffer;
    stat(filename, &stat_buffer);
    return S_ISDIR(stat_buffer.st_mode);
}



/**************************************************
 * function to read contents of file into a string and return it.
 * Inputs: 
 *      - char *, name of file to get contents of
 * Outputs:
 *      - char *, string containing contents of file.
 *        if error occurs reading file, returned char * is NULL
 * Based on example found here:
 * https://www.includehelp.com/c-programs/find-size-of-file.aspx
**************************************************/
char *Server::get_file_contents(char *filename)
{
    char *contents = NULL;
    int file_len;
    
    // open file
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
        // move to end of file, get the position of file pointer
        fseek(fp, 0, SEEK_END);
        file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (file_len != -1)
        {
            contents = new char[file_len + 1];

            //fgets(contents, file_len + 1, fp);
            fread(contents, 1, file_len, fp);
            contents[file_len] = '\0';
        }
    }
    fclose(fp);
    return contents;
}


/**************************************************
 * function to handle file transfer command. makes sure requested file
 * exists and is not a directory. gets contents of file, opens data connection
 * to client, and sends file.
 * Inputs:
 *      - int &, actively connected socket fd, used for command communication
 *      - char *, name of connected host
 *      - char *, port number for data transfer connection
 *      - char *, name of requested file
 * Outputs:
 *      - bool, true if file transferred successfully, false if not
**************************************************/
bool Server::transfer_file(Socketft *client, char *data_port, char *file)
{

    char *host = client->getHost();
    // check if file exists in current directory
    if (!valid_filename(file))
    {
        // file not found, send error message on command socket
        printf("File not found. Sending error message to %s:%s\n", host, port);
        fflush(stdout);

        // check for success sending
        if (!client->send_message("ERROR: file not found"))
            fprintf(stderr,"ERROR: unable to send status message to client command socket\n");
            fflush(stderr);
        // file not transferred
        return false;
    }
    // check if filename is a directory
    else if (is_directory(file))
    {
        // file is a directory, send error message on command socket
        printf("\"%s\" is a directory. Sending error message to %s:%s\n", file, host, port);
        fflush(stdout);

        // check for success sending
        string err_msg = "ERROR: \"" + string(file) + "\" is a directory";
        if (!client->send_message(err_msg.c_str()))
            fprintf(stderr,"ERROR: unable to send status message to client command socket\n");
            fflush(stderr);

        // file not transferred
        return false;
    }

    // call function to get pointer to string containing file contents
    char *contents = get_file_contents(file);

    // check for error
    if (contents == NULL)
    {
        printf("Unable to read file. Sending error message to %s:%s\n", host, port);
        
        // check for success sending
        if (!client->send_message("ERROR: unable to read file"))
            fprintf(stderr,"ERROR: unable to send status message to client command socket\n");
            fflush(stderr);
        // file not transferred
        return false;
    }

    // send OK status message on command socket
    if (!client->send_message("OK"))
    {
        // OK message failed
        fprintf(stderr,"ERROR: unable to send status message to client command socket\n");
        fflush(stderr);

        // file not transferred
        delete contents;
        return false;
    }

    // open connection to client on data port, send content string
    Socketft data_socket(data_port, host);

    // check connection was successful
    if (data_socket.open_connection())
    {
        printf("Sending \"%s\" to %s:%s\n", file, host, data_port);
        fflush(stdout);
        if (!data_socket.send_message(contents))
        {
            fprintf(stderr, "ERROR: unable to transfer file to %s:%s\n", host, data_port);
            fflush(stderr);
            delete contents;
            data_socket.close_socket();
            return false;
        }
        
        delete contents;
        data_socket.close_socket();
        return true;
    }

    // connection to data socket failed
    delete contents;
    return false; 
}
