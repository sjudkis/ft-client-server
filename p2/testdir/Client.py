from socket import *
import sys
import os.path

class Client:
    """implementation of Client class"""
    

    # function to validate command line args. Port numbers must be numbers,
    # cannot be identical. command must be -l or -g
    # Inputs:
    #       - array containing command line args
    # Output:
    #       - True if args are valid. args assigned to instance variables
    #           - host_name
    #           - command_port
    #           - command
    #           - data_port
    #           - filename (if command == -g)
    #       - False if args not valid, prints error message
    def validate_args(self, args):
        # must have at least 5 args
        if len(args) < 5:
            return False

        # assign host arg to instance variable
        if args[1] in ("flip1", "flip2", "flip3"):
            self.host_name = args[1] + ".engr.oregonstate.edu"
        else:
            self.host_name = args[1]

        # make sure server port is a number
        try:
            self.command_port = int(args[2])
        except:
            print("ERROR: invalid server port number", file=sys.stderr)
            return False

        # check for valid command
        self.command = args[3]
        
        if self.command == "-l":

            # list command, must be followed by data port number
            try:
                # attempt to convert port arg to int
                self.data_port = int(args[4])
            except:
                return False

            # check if too many arguments
            if len(args) > 5:
                return False

        elif self.command == "-g":
            # list command, must be followed by filename and data port number
            
            # assign filename arg to instance variable
            self.filename = args[4]
            
            try:
                # attempt to convert port arg to int
                self.data_port = int(args[5])
            except:
                print("ERROR: invalid data port number", file=sys.stderr)
                return False
            
            # check if too many arguments
            if len(args) > 6:
                return False
        else:
            print("ERROR: invalid command", file=sys.stderr)
            return False
        # args are valid, return True

        # make sure command and data ports are different
        if self.command_port == self.data_port:
            print("ERROR: server port and data port numbers must be different", file=sys.stderr)
            return False

        # args are valid and have been assigned to instance variables
        return True



    # function to connect to server on command port. 
    # input:
    #       - none
    # output:
    #       - stores connected socket in instance variable
    def connect_to_server(self):
        self.commandfd = create_connection((self.host_name, self.command_port))


    # function to send command to to server. builds command string from args
    # stored in instances variables, based on the command passed in from 
    # command line. format: <command_len>$<command args separated by spaces>
    # input:
    #       - none, uses instance variables
    # output:
    #       - no return value, command message is sent to server
    def send_command(self):
        # build string depending on command
        if self.command == "-l":
            command_string = f"{self.command} {str(self.data_port)}"
        elif self.command == "-g":
            command_string = f"{self.command} {self.filename} {str(self.data_port)}"
        
        # build complete message to send, including length of command string
        total_message = f"{len(command_string)}${command_string}"

        self.commandfd.sendall(total_message.encode())


    # function to create an actively listening socket to receive data 
    # sent by server.
    # input:
    #       - none, uses instance variable, data_port
    # output:
    #       - no return value. actively listening socket stored in instance var
    #          called datafd.
    def create_data_socket(self):
        self.datafd = socket(AF_INET, SOCK_STREAM)
        self.datafd.bind(('', self.data_port))
        self.datafd.listen(1)
        return

    
    # function to receive a message from server. parses message to extract length, 
    # ensures entire message is received.
    # input:
    #       - active socket to read from 
    # output:
    #       - returns received message, or an error message to print
    def receive_message(self, fd):
        # receive first portion of message
        received = fd.recv(1024)

        # check for error recceiving message
        error = f"ERROR: connection with server has been broken"
        if received == b'':
            return error
        
        # decode message, parse message length
        received_string = received.decode()
        delim = received_string.index("$")
        message_len = int(received_string[:delim])

        #store message portion received, excluding message length and $
        message = received_string[delim + 1:]

        # continue receiving until entite message arrives
        bytes_received = len(message)
        while bytes_received < message_len:
            # receive next portion of message
            received = fd.recv(1024)
            if received == b'':
                return error

            # decode message portion
            received_string = received.decode()

            bytes_received += len(received_string)
            message += received_string

        return message


    
    # function to receive directory listing from server.
    # input:
    #       - none, uses instance variable, datafd
    # output:
    #       - no return value, prints either directory contents or error 
    #         message 
    def handle_directory_info(self):
        # accept data transfer connection
        datafd, addr = self.datafd.accept()
        print(f"Receiving directory structure from {self.host_name}:{self.data_port}")
        
        # receive data from server
        contents = self.receive_message(datafd)
        
        # check if error receiving
        if contents[:5] == "ERROR":
            print(contents,file=sys.stderr)
        else:
            print(contents)
        
        datafd.close()


    # function to receive file data from server. if matching filename
    # already exists in client's directory, asks user if they want to 
    # replace it.
    # input:
    #       - none, uses instance variables
    # output:
    #       - no return value, if successful, new file is create with
    #         received data. 
    # code to check if file exists based on:
    # https://stackoverflow.com/questions/82831/how-do-i-check-whether-a-file-exists-without-exceptions
    def handle_file_transfer(self):
        datafd, addr = self.datafd.accept()
        print(f"Receiving \"{self.filename}\" from {self.host_name}:{self.data_port}")
        contents = self.receive_message(datafd)
        if contents[:5] == "ERROR":
            print(contents,file=sys.stderr)
        else:
            # if file doesn't exist or user agrees to replace it, wrtie file
            if not os.path.exists(self.filename) or self.replace_file():
                new_file = open(self.filename, "w")
                new_file.write(contents)
                new_file.close()
                print("File transfer complete")
        
        datafd.close()
        return
        
    # function to ask user if they want to replace file. Prompts until they 
    # enter Y or N, case insensitive
    # input:
    #       - none
    # output:
    #       - true or false, depending on user's answer
    def replace_file(self):
        while (True):
            res = input(f"\"{self.filename}\" already exists. Do you want to replace it? (Y/N): ")
            if res.strip().upper() == "Y":
                return True
            elif res.strip().upper() == "N":
                return False
