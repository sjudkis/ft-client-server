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
#include "Socketft.hpp"

Socketft::Socketft(char* p, char* h)
{
    port = p;
    host = h;
}

Socketft::Socketft(char* h, int f)
{
    port = nullptr;
    host = h;
    fd = f;
}

bool Socketft::start_listening()
{
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo; // holds server address info

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // create TCP socket
    hints.ai_flags = AI_PASSIVE;        // fill in IP for me

    // get address info for current host and port num
    status = getaddrinfo(NULL, port, &hints, &servinfo);

    // check for success
    if (status < 0)
    {
        fprintf(stderr, "ERROR: Unable to find address info for port %s\n", port);
        fflush(stderr);
        return false;
    }

    fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (fd < 0)
    {
        fprintf(stderr, "ERROR: unable to open socket\n");
        fflush(stderr);
        return false;
    }

    // bind socket to port
    status = bind(fd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status < 0)
    {
        fprintf(stderr, "ERROR: Unable to bind on port %s\n", port);
        fflush(stderr);
        return false;
    }

    // start listening on socket
    status = listen(fd, 5);
    if (status < 0)
    {
        fprintf(stderr, "ERROR: Unable to listen port %s\n", port);
        fflush(stderr);
        return false;
    }

    // successfully listening, free memory and return true
    freeaddrinfo(servinfo);
    return true;
}

bool Socketft::open_connection(){
    // create structs for address info
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // use TCP socket
    hints.ai_flags = AI_PASSIVE;

    // get address info for destination host
    getaddrinfo(host, port, &hints, &res);

    // create socket
    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0)
    {    
        fprintf(stderr, "ERROR: unable to create data socket\n");
        fflush(stderr);
        freeaddrinfo(res);
        return false;
    }

    // connect socket to server
    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0)
    {
        fprintf(stderr, "ERROR: unable to start data transfer connection with host %s:%s\n", 
                host, port);
        fflush(stderr);
        freeaddrinfo(res);
        return false;
    }

    freeaddrinfo(res);

    // successfully connected to client's data transfer socket
    return true;
}

Socketft Socketft::accept_connection()
{
    // struct to hold client address info
    struct sockaddr_storage client_addr; // holds client address info
    socklen_t addr_size = sizeof(client_addr);

    // accept client connection
    int newfd = accept(fd, (struct sockaddr *)&client_addr, &addr_size);

    // extract host name of connected client
    char connected_host[1024];
    getnameinfo((struct sockaddr *)&client_addr, addr_size, connected_host, 
                sizeof(connected_host), NULL, 0, 0);

    char *host_name = new char[strlen(connected_host) + 1];
    strcpy(host_name, connected_host);

    Socketft newConn = Socketft(host_name, newfd);
    return newConn;
}

char *Socketft::recv_message()
{
    char read_buff[1025];
    memset(read_buff, '\0', sizeof(read_buff));

    // read from new socket
    int bytes_read = recv(fd, read_buff, 1024, 0);
    if (bytes_read < 0)
        return nullptr;
        
    char *delim_char = strstr(read_buff, "$");
    *delim_char = '\0';

    //convert size string to int. size of actual user message
    int message_size = atoi(read_buff);

    // create char array to hold complete message string, plus null terminator
    char *message = new char[message_size + 1];
    memset(message, '\0', message_size + 1);

    // copy received portion of message into string
    strcpy(message, delim_char + 1);

    // count all bytes in message so far
    int total_read = strlen(message);

    // make sure entire message is received
    while (total_read < message_size)
    {
        memset(read_buff, '\0', sizeof(read_buff));
        bytes_read = recv(fd, read_buff, 1024, 0);

        // check for error or closed connection
        if (bytes_read <= 0)
        {
            delete(message);
            return nullptr;
        }
    
        // add to total amount received, append new portion of message
        total_read += strlen(read_buff);
        strcat(message, read_buff);
    }

    return message;
}


/* format of message: <message_length>$<message_text>
 * prepends message with length of the message portion, so receiver 
 * knows how much data to expect. length and text separated by "$"
 */
 bool Socketft::send_message(char *message)
 {
    int msg_len = strlen(message);  // length of message portion
    char len_buf[20];               // hold string containing msg_len chars
    memset(len_buf, '\0', 20);
    
    // convert msg_len into string stored in len_buf
    sprintf(len_buf, "%d", msg_len);

    int total_len = strlen(len_buf) + msg_len + 1;
    char *complete_message = new char[total_len + 1];
    strcpy(complete_message, len_buf);
    strcat(complete_message, "$");
    strcat(complete_message, message);

    // attempt to send message to socket, check for error
    int total_sent = send(fd, complete_message, total_len, 0);
    if (total_sent <= 0)
        return false;

    // make sure entire message was sent
    int check_send = -19;
    do 
    {
        ioctl(fd, TIOCOUTQ, &check_send); 
    }
    while (check_send > 0);
    if (check_send < 0) 
        return false;

    // message sent successfully
    delete complete_message;
    return true;
 }