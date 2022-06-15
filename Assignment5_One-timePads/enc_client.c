#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h> // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <sys/stat.h>
#include <ctype.h>
#define SIZE 70000

// Send a file
void sendFile(char* filename, int socketFD, int length) {
    FILE * temp = fopen(filename, "r");
    char buffer[SIZE];
    bzero(buffer, SIZE); // clear the buffer
    int bytesSent;
    
    // while the file is read, send each byte
    while ((length = fread(buffer, sizeof(char), SIZE, temp)) > 0) {
        if ((bytesSent = send(socketFD, buffer, length, 0)) < 0) {
            break;
        }
        bzero(buffer, SIZE);
    }
    
    if (bytesSent == SIZE) {
        send(socketFD, "0", 1, 0);
    }
    fclose(temp);
    return;
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHost;
    char buffer[SIZE];
    char authenticate[] = "e";
    bzero(buffer, sizeof(buffer));
    charsWritten = 1;
    
    // the last number will be user's port number, set up the socket
    portNumber = atoi(argv[3]);
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        perror("Error: opening socket");
    }
    
    // find the host information and store
    serverHost = gethostbyname("localhost");
    if (serverHost == NULL) {
        fprintf(stderr, "Error: no such host");
        exit(0);
    }
    
    // sets the socket options, make it resusable
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &charsWritten, sizeof(int));
    
    bzero((char*)&serverAddress, sizeof(serverAddress)); // clear the server address
    serverAddress.sin_family = AF_INET; // network capable
    bcopy((char*)serverHost->h_addr, (char*)&serverAddress.sin_addr.s_addr, serverHost->h_length); // copy in the address
    serverAddress.sin_port = htons(portNumber);
    
    // check the connection
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error connecting");
        exit(1);
    }
    
    // write and read
    write(socketFD, authenticate, sizeof(authenticate));
    read(socketFD, buffer, sizeof(buffer));
    if (strcmp(buffer, "e") != 0) {
        fprintf(stderr, "Error, cannot use");
        exit(2);
    }
    
    // get the sizes of the key and plaintext
    int plaintext = open(argv[1], O_RDONLY);
    int psize = lseek(plaintext, 0, SEEK_END);
    int key = open(argv[2], O_RDONLY);
    int ksize = lseek(key, 0, SEEK_END);
    
    // if the key is smaller than the plaintext, reprompt for bigger key
    if (ksize < psize) {
        fprintf(stderr, "Error: key is too short");
        exit(1);
    }
    
    // check for bad characters in the plaintext
    int badChar = open(argv[1], 'r');
    while (read(badChar, buffer, 1) != 0) {
        if (isspace(buffer[0]) || isalpha(buffer[0])) {
            
        } else {
            fprintf(stderr, "Bad characters in %s\n", argv[1]);
            exit(1);
        }
    }
    
    bzero(buffer, sizeof(buffer)); // clear buffer
    
    // send the plaintext and the key
    sendFile(argv[1], socketFD, psize);
    sendFile(argv[2], socketFD, ksize);
    
    charsRead = read(socketFD, buffer, sizeof(buffer)-1);
    if (charsRead < 0) {
        perror("Error: reading from socket");
        exit(1);
    }
    printf("%s\n", buffer);
    close(socketFD); // Close the socket
    return 0;
}
