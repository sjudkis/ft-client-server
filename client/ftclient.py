# Program Name: ftclient
# Author: Sam Judkis
# Description:
#   main file for file transfer client, using my original Client class.
#   accepts command line argument in two ways:
#       1. list directory:
#           ftclient.py <SERVER_HOST> <SERVER_PORT> -l <DATA_PORT>
#       2. file transfer:
#           ftclient.py <SERVEr_HOST> <SERVER_PORT> -g <FILENAME> <DATA_PORT>
#   
#   validates command arguments, connects to server, and sends command.
#   gets a command status message back from server, if message is "OK", opens
#   new data transfer socket and accepts connection from server, then receives 
#   the data that was requested. prints any error message send by server.
#
#   Python socket programming was mostly based on the Python documentation:
#       https://docs.python.org/3.6/howto/sockets.html
#       https://docs.python.org/3.6/library/socket.html?highlight=socket#module-socket
#   


import sys
from socket import *
from Client import *


def main():
    client = Client()
    if not client.validate_args(sys.argv):
        print("USAGE: ftclient.py <SERVER_HOST> <SERVER_PORT#> " + \
                "<COMMAND> [FILENAME] <DATA_PORT#>", file=sys.stderr)
        return 1

    # attempt to connect to server on command port
    try:
        client.connect_to_server()
    except:
        print(f"ERROR: unable to connect to server on port {client.command_port}", file=sys.stderr)
        return 2

    # attempt to send command message to server on command socket
    try:
        client.send_command()
    except:
        print(f"ERROR: unable to send command to server on port {client.command_port}", file=sys.stderr)
        client.commandfd.close()
        return 3

    # receive command status message from server
    try:
        command_status = client.receive_message(client.commandfd)
    except:
        print(f"ERROR: unable to receive from server on port {client.command_port}", file=sys.stderr)
        client.commandfd.close()
        return 4
    
    # if OK received, command is valid, prepare to receive data through data
    # port socket
    if command_status == "OK":
        # create data transfer socket
        try:
            client.create_data_socket()
        except:
            print(f"ERROR: unable to create data socket on port {client.data_port}", file=sys.stderr)
            client.commandfd.close()
            return 5

        if client.command == "-l":
            # call function to receive directory data
            client.handle_directory_info()
        elif client.command == "-g":
            # call function to receive file data
            client.handle_file_transfer()
    
    else:
        # otherwise, there is an error, print received message and exit
        err_msg = f"{client.host_name}:{client.command_port} says \'{command_status}\'"
        print(err_msg, file=sys.stderr)
        client.commandfd.close()
        return 6
    
    client.commandfd.close()
    return




if __name__ == '__main__':
    main()
