#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Function to create and connect the TCP socket to the server
int connectToServer(const char* serverIp, const char* serverPort) {
    int clientSocketFd;
    struct sockaddr_in serverAddress;

    if ((clientSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Create socket
        perror("Could not create socket.");
        exit(1);
    }

    memset(&serverAddress, 0, sizeof(serverAddress)); // Set up address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(serverPort));
    if (inet_pton(AF_INET, serverIp, &serverAddress.sin_addr) != 1) {
        perror("Invalid IP address.");
        exit(1);
    }

    if (connect(clientSocketFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connect Failed.");
        exit(1);
    }

    return clientSocketFd;
}

// Function to send the file size to the server
void sendFileSize(int clientSocketFd, uint32_t fileByteCount) {
    int totalBytesSent = 0;
    int bytesRemaining = 4;
    int currentBytesSent;

    // Send the file size to the server
    uint32_t fileSizeForNetworkTransmission = htonl(fileByteCount);
    while (bytesRemaining > 0) {
        currentBytesSent = write(clientSocketFd, (char *) &fileSizeForNetworkTransmission + totalBytesSent, bytesRemaining);
        if (currentBytesSent <= 0) {
            close(clientSocketFd);
            perror("Failed to send file size to the server.");
            exit(1);
        }
        totalBytesSent += currentBytesSent;
        bytesRemaining -= currentBytesSent;
    }
}

// Function to send the file content to the server
void sendFileContent(int clientSocketFd, int fileDescriptor, uint32_t fileByteCount) {
    int totalBytesSent = 0;
    int bytesRemaining = fileByteCount;
    int currentBytesRead;
    int currentBytesSent;
    int messageChunkSize;
    char buffer[1000000]; // Buffer with less than 1MB

    // Send the file content to the server
    while (bytesRemaining > 0) {
        if (sizeof(buffer) < bytesRemaining) {
            messageChunkSize = sizeof(buffer);
        } else {
            messageChunkSize = bytesRemaining;
        }
        currentBytesRead = read(fileDescriptor, buffer, messageChunkSize);
        if (currentBytesRead <= 0) {
            close(clientSocketFd);
            perror("Failed to read file content.");
            exit(1);
        }
        currentBytesSent = write(clientSocketFd, buffer, currentBytesRead);
        if (currentBytesSent <= 0) {
            close(clientSocketFd);
            perror("Failed to send file content to the server.");
            exit(1);
        }
        totalBytesSent += currentBytesSent;
        bytesRemaining -= currentBytesSent;
        lseek(fileDescriptor, totalBytesSent, SEEK_SET);
    }
}

// Function to receive the number of printable characters from the server
uint32_t receiveCharacterCount(int clientSocketFd) {
    int totalBytesSent = 0;
    int bytesRemaining = 4;
    int currentBytesSent;
    uint32_t printableCharacterCount;

    // Receive the nuber of printable characters from the server
    while (bytesRemaining > 0) {
        currentBytesSent = read(clientSocketFd, (char *) &printableCharacterCount + totalBytesSent, bytesRemaining);
        if (currentBytesSent <= 0) {
            close(clientSocketFd);
            perror("Failed to receive printable character count from the server.");
            exit(1);
        }
        totalBytesSent += currentBytesSent;
        bytesRemaining -= currentBytesSent;
    }

    printableCharacterCount = ntohl(printableCharacterCount);
    return printableCharacterCount;
}

int main(int argc, char *argv[]) {
    // Verify command line arguments
    if (argc != 4) {
        perror("Usage: pcc_client <server_ip> <server_port> <file_to_send>");
        exit(1);
    }

    // Open the file using open() system call:
    int fileDescriptor = open(argv[3], O_RDONLY);
    if (fileDescriptor == -1) {
        perror("Couldn't open file.");
        exit(1);
    }

    // Create the TCP connection to the server
    int clientSocketFd = connectToServer(argv[1], argv[2]);

    // Determine the file size using fstat()
    struct stat fileStat;
    if (fstat(fileDescriptor, &fileStat) < 0) {
        perror("fstat Failed.");
        exit(1);
    }
    uint32_t fileByteCount = fileStat.st_size;

    sendFileSize(clientSocketFd, fileByteCount); // Send the file size to the server
    sendFileContent(clientSocketFd, fileDescriptor, fileByteCount); // Send the file content to the server
    close(fileDescriptor);
    uint32_t printableCharacterCount = receiveCharacterCount(clientSocketFd); // Receive the number of printable characters from the server

    close(clientSocketFd);
    printf("# of printable characters: %u\n", printableCharacterCount);
    exit(0);
}