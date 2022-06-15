#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define SIZE 70000

char inttochar(int num) {
    static const char * chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    if (num < 0 || num > 27) {
        return 'a';
    }
    return chars[num];
}

int chartoint(char c) {
    static const char * chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    for (int i = 0; i < 27; i++) {
        if (c == chars[i]) {
            return i;
        }
    }
    return -1;
}

void decrypt(char message[], char key[], int length) {
    int i, textNum, keyNum, decryptNum;
    length = strlen(message) - 1;
    for (i = 0; i < length; i++) {
        textNum = chartoint(message[i]);
        keyNum = chartoint(key[i]);
        decryptNum = (textNum - keyNum) % 27;
        if(decryptNum < 0) { // case of negative
            decryptNum += 27;
        }
        message[i] = inttochar(decryptNum);
    }
    message[i] = '\0';
    return;
}


int main(int argc, char *argv[]) {
    int socketFD, portNumber, val, charsWritten, newSocketFD;
    char buffer[SIZE];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t child_pid;
    charsWritten = 1;

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        perror("Error opening socket");
        exit(1);
    }
    
    // sets the socket options, make it resusable
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &charsWritten, sizeof(int));
    
    bzero((char*)&serverAddress, sizeof(serverAddress)); // clear the server address
    portNumber = atoi(argv[1]);
    serverAddress.sin_family = AF_INET; // network capable
    serverAddress.sin_port = htons(portNumber); // save the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // allow any address

    // Associate the socket to the port
    if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        perror("Error on binding");
        exit(1);
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(socketFD, 5);

    // Accept a connection, blocking if one is not available until one connects
    while(1) {
        // Accept the connection request which creates a connection socket
        newSocketFD = accept(socketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if (newSocketFD < 0){
            perror("Error on accept");
            exit(1);
        }

        // METHOD 1: Create a new process with fork when the connection is made.
        child_pid = fork();
        if (child_pid < 0) { // if fork failed
            perror("Error on fork()");
            exit(1);
        }
        if (child_pid == 0) { // the fork succeeded
            bzero(buffer, sizeof(buffer));
            char * key;
            int next, numBytes, bytesRead;
            numBytes = sizeof(buffer);
            bytesRead = 0;
            next = 0;
            
            read(newSocketFD, buffer, sizeof(buffer) - 1); // try read, exit if unable
            if (strcmp(buffer, "dec") != 0) {
                char output[] = "not valid";
                write(newSocketFD, output, sizeof(output));
                exit(2);
            } else {
                char output[] = "dec";
                write(newSocketFD, output, sizeof(output));
            }
            bzero(buffer, sizeof(buffer)); // clear the buffer and use a pointer
            char* buffHelper = buffer;
            
            while (1) {
                bytesRead = read(newSocketFD, buffHelper, numBytes);
                if (numBytes == 0) { // no bytes, so don't read
                    break;
                }
                
                // iterate to find new lines
                for (int iter = 0; iter < bytesRead; iter++) {
                    if (buffer[iter] == '\n') {
                        ++next;
                        if (next == 1) {
                            key = buffer + iter + 1;
                        }
                    }
                }
                
                if (next == 2) {
                    break;
                }
                
                numBytes = numBytes - bytesRead; // subtract the number of bytes
                buffHelper = buffHelper + bytesRead;
            }
            char message[SIZE];
            bzero(message, sizeof(message));
            strncpy(message, buffer, key - buffer); // copy
            decrypt(message, key, strlen(message)); // encrypt
            write(newSocketFD, message, sizeof(message)); // write to the socket
        }
        close(newSocketFD);
    }
    // Close the listening socket
    close(socketFD);
    return 0;
}
