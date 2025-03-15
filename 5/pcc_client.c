#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

/*Support functions*/
int establish_connection(const char *ip_address, const char *port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Failed createing socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, ip_address, &server_address.sin_addr) != 1) {
        perror("Failed inet_pton.\n");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to connecec.\n");
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

uint32_t get_file_size(int file_descriptor) {
     /* Seek to the end of the file to determine  */
    uint32_t file_size = lseek(file_descriptor, 0, SEEK_END);
    if (file_size == (uint32_t) -1) {
        perror("Failed lseek.\n");
        close(file_descriptor);
        exit(EXIT_FAILURE);
    }

    if (lseek(file_descriptor, 0, SEEK_SET) == (off_t) -1) {
        perror("failed lseek.\n");
        close(file_descriptor);
        exit(EXIT_FAILURE);
    }

    return file_size;
}

void send_data(int socket_fd, const void *data, size_t data_size) {
    
    size_t total_sent = 0;
      /* Send the file contents granurailily */
    while (total_sent < data_size) {
        ssize_t bytes_sent = write(socket_fd, (const char *)data + total_sent, data_size - total_sent);
        if (bytes_sent <= 0) {
            perror("Failed sending data.\n");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
        total_sent += bytes_sent;
    }
}

void send_file(int socket_fd, int file_fd, uint32_t file_size) {
    size_t total_sent = 0;
    char buffer[1000000];

    while (total_sent < file_size) {
        ssize_t bytes_read = read(file_fd, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("Failed to read file.\n");
            close(socket_fd);
            close(file_fd);
            exit(EXIT_FAILURE);
        }

        send_data(socket_fd, buffer, bytes_read);
        total_sent += bytes_read;
    }
}

uint32_t receive_data(int socket_fd) {
    size_t total_received = 0;
    uint32_t data;
    while (total_received < sizeof(data)) {
        ssize_t bytes_received = read(socket_fd, (char *) &data + total_received, sizeof(data) - total_received);
        if (bytes_received <= 0) {
            perror("Failed to get recieve server.\n");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
        total_received += bytes_received;
    }
    return ntohl(data);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        perror("Usage: <server IP> <server port> <file path>\n");
        exit(EXIT_FAILURE);
    }

    int file_fd = open(argv[3], O_RDONLY);
    if (file_fd == -1) {
        perror("Failed to open.\n");
        exit(EXIT_FAILURE);
    }

    int socket_fd = establish_connection(argv[1], argv[2]);

    uint32_t file_size = get_file_size(file_fd);
    uint32_t file_size_network_order = htonl(file_size);

    send_data(socket_fd, &file_size_network_order, sizeof(file_size_network_order));
    send_file(socket_fd, file_fd, file_size);

    close(file_fd);

    uint32_t printable_characters_count = receive_data(socket_fd);
    printf("# of printable characters: %u\n", printable_characters_count);

    close(socket_fd);
    return 0;
}

