/******************************************************
 * Program Name: ftserver
 * Author: Sam Judkis
 * Last Modified: 6/2/19
 * Description:
 *      main file for the file transfer server, using my original
 *      Server class. Accepts one command line arg, giving the port number
 *      to listen on. Validates port number, then starts server.
 *      Loops continuously to accept new clients, using Server member
 *      function. Runs until terminated by SIGINT. 
 *      Clients can request a listing of the contents of the directory in
 *      which the Server is running, or the transfer of a file within that 
 *      directory. Server validates these commands and arguments and fulfills
 *      the request if valid, sending error messages to client if not valid.
 *      Command communication and data transfer occur on different ports
 *      C socket programming was mostly based on Beej's Guide to Network Programming, 
 *      found here:
 *      https://beej.us/guide/bgnet/html/multi/index.html
 * ***************************************************/

#include "Server.hpp"
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <cctype>
#include <cstring>

using std::cout;
bool valid_port(char*);

int main(int argc, char* argv[])
{
    // check number of arguments
    if (argc != 2)
    {
        fprintf(stderr, "usage: ./ftserver <port#>\n");
        fflush(stderr);
        return 1;
    }
    // make sure arg is a number
    if (!valid_port(argv[1]))
    {
        fprintf(stderr, "usage: ./ftserver <port#>\n");
        fflush(stderr);
        return 1;
    }

    // create a Server, passing in port number
    Server server(argv[1]);
    
    // start server, returns true if successful, false if failed
    if (server.start_server())
        printf("Server now listening on port %s...\n", server.get_port());
    else
        // server failed to start, exit with error code 1
        return 1;

    // continuously loop to accept client connections
    while (true)
    {
        // call member function to handle a client connection
        server.handle_client();
    }

    return 0;
}

// function to validate a given string containing a port number. 
// Ensures each char in string is a number. Returns true if so,
// false otherwise
bool valid_port(char *port)
{
    int plen = strlen(port);
    for (int i = 0; i < plen; i++)
    {
        // check that every char in port string is a digit
        if (!isdigit(port[i]))
            return false;
    }
    return true;
}
