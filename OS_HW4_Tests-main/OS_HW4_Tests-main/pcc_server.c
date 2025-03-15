#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

uint32_t printableCharacterCounts[95];
int currentClientConnectionFd = -1;
int isWaitingForClients = 1;

// Function to create and bind the TCP server
int createAndBindTCPServer(const char* serverPort) {
    int serverListenFd;
    const int enableReuseAddress = 1;
    struct sockaddr_in serverAddress;

    // Create the TCP socket
    if ((serverListenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Couldn't make socket.");
        exit(1);
    }

    if (setsockopt(serverListenFd, SOL_SOCKET, SO_REUSEADDR, &enableReuseAddress, sizeof(int)) < 0) {
        perror("setsockopt failed.");
        exit(1);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(atoi(serverPort));
    if (bind(serverListenFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) != 0) { // Binder
        perror("Bind Failed.");
        exit(1);
    }
    if (listen(serverListenFd, 10) != 0) { // Listen
        perror("Listen Failed.");
        exit(1);
    }

    return serverListenFd;
}

// Print the statistics of printable characters
void printPrintableCharacterCounts() {
    int i;
    for (i = 0; i < 95; i++) {
        printf("char '%c' : %u times\n", (char) (i + 32), printableCharacterCounts[i]);
    }
    exit(0);
}

// Signal handler fro SIGINT
void handleSigintSignal(int signum) {
    if (currentClientConnectionFd == -1) {
        printPrintableCharacterCounts();
    }
    isWaitingForClients = 0;
}



// Function to receive the file size from the client
uint32_t receiveFileSize(int currentClientConnectionFd) {
    int totalBytesSent = 0;
    int bytesRemaining = 4;
    int currentBytesSent;
    uint32_t fileByteCount;

    // Receive the file size from the client
    while (bytesRemaining > 0) {
        currentBytesSent = read(currentClientConnectionFd, (char *) &fileByteCount + totalBytesSent, bytesRemaining);
        if (currentBytesSent == 0 || (currentBytesSent < 0 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
            perror("Failed to receive file size.");
            close(currentClientConnectionFd);
            bytesRemaining = 0;
            currentClientConnectionFd = -1;
        } else if (currentBytesSent < 0) {
            perror("Failed to receive file size from the client (Unknown).");
            close(currentClientConnectionFd);
            exit(1);
        } else {
            totalBytesSent += currentBytesSent;
            bytesRemaining -= currentBytesSent;
        }
    }
    if (currentClientConnectionFd == -1) {
        return 0;
    }
    return ntohl(fileByteCount);
}


void receiveFileContentAndCalculateStatistics(int currentClientConnectionFd, uint32_t fileByteCount,
                                             uint32_t* printableCharacterCount, uint32_t tempPrintableCharacterCounts[]) {
    int totalBytesSent = 0;
    int bytesRemaining = fileByteCount;
    int currentBytesSent;
    int i;
    char buffer[1000000];

    // Receive the file content from the client and calculate statistics
    memset(tempPrintableCharacterCounts, 0, 95 * sizeof(uint32_t));
    *printableCharacterCount = 0;
    while (bytesRemaining > 0) {
        int messageChunkSize = (sizeof(buffer) < bytesRemaining) ? sizeof(buffer) : bytesRemaining;
        currentBytesSent = read(currentClientConnectionFd, (char *) &buffer, messageChunkSize);
        if (currentBytesSent == 0 || (currentBytesSent < 0 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
            perror("Failed to receive file content.");
            close(currentClientConnectionFd);
            bytesRemaining = 0;
            currentClientConnectionFd = -1;
        } else if (currentBytesSent < 0) {
            perror("Failed to receive file content (Unkown).");
            close(currentClientConnectionFd);
            exit(1);
        } else {
            for (i = 0; i < currentBytesSent; i++) {
                if (buffer[i] >= 32 && buffer[i] <= 126) {
                    tempPrintableCharacterCounts[(int) (buffer[i]) - 32]++;
                    (*printableCharacterCount)++;
                }
            }
            totalBytesSent += currentBytesSent;
            bytesRemaining -= currentBytesSent;
        }
    }
}

// Function to send the number of printable characters to the client
void sendCharacterCount(int currentClientConnectionFd, uint32_t printableCharacterCount) {
    int totalBytesSent = 0;
    int bytesRemaining = 4;
    int currentBytesSent;

    // Send the number of printable characters to the client
    printableCharacterCount = htonl(printableCharacterCount);
    while (bytesRemaining > 0) {
        currentBytesSent = write(currentClientConnectionFd, (char *) &printableCharacterCount + totalBytesSent, bytesRemaining);
        if (currentBytesSent == 0 || (currentBytesSent < 0 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
            perror("Failed to send printable character count.");
            close(currentClientConnectionFd);
            bytesRemaining = 0;
            currentClientConnectionFd = -1;
        } else if (currentBytesSent < 0) {
            perror("Failed to send printable character count (Unknown).");
            close(currentClientConnectionFd);
            exit(1);
        } else {
            totalBytesSent += currentBytesSent;
            bytesRemaining -= currentBytesSent;
        }
    }
}

// Function to update the global printable character counts
void updateGlobalPrintableCharacterCounts(uint32_t tempPrintableCharacterCounts[]) {
    int i;
    for (i = 0; i < 95; i++) {
        printableCharacterCounts[i] += tempPrintableCharacterCounts[i];
        tempPrintableCharacterCounts[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    // Verify command line arguments
    if (argc != 2) {
        perror("Usage: pcc_server <server_port>");
        exit(1);
    }

    struct sigaction newSigintAction = {
        .sa_handler = handleSigintSignal,
        .sa_flags = SA_RESTART
    };
    if (sigaction(SIGINT, &newSigintAction, NULL) == -1) {
        perror("Failed to register signal handler.");
        exit(1);
    }

    // Create the TCP server
    int serverListenFd = createAndBindTCPServer(argv[1]);

    // Main server loop
    while (isWaitingForClients) {
        currentClientConnectionFd = accept(serverListenFd, NULL, NULL); // Accept a client connection
        if (currentClientConnectionFd < 0) {
            perror("Accept Failed.");
            exit(1);
        }
        uint32_t fileByteCount = receiveFileSize(currentClientConnectionFd);
        if (currentClientConnectionFd == -1) {
            continue;
        }

        // Receive the file content from the client and calculate statistics
        uint32_t printableCharacterCount;
        uint32_t tempPrintableCharacterCounts[95];

        receiveFileContentAndCalculateStatistics(currentClientConnectionFd, fileByteCount, &printableCharacterCount, tempPrintableCharacterCounts);
        if (currentClientConnectionFd == -1) { continue; }

        sendCharacterCount(currentClientConnectionFd, printableCharacterCount); // Sends charcount to client
        if (currentClientConnectionFd == -1) { continue; }

        updateGlobalPrintableCharacterCounts(tempPrintableCharacterCounts);
        close(currentClientConnectionFd);
        currentClientConnectionFd = -1;
    }
    printPrintableCharacterCounts(); // Will exit properly.
}