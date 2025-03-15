#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

/*declerations */
void handle_SIGINT(int signum);
void display_statistics();
void setup_server(int *server_fd, const char *port);
void handle_client_connection(int client_fd);
void receive_data(int client_fd, uint32_t *file_size, uint32_t *printable_count, uint32_t temp_pcc_total[95]);
void send_printable_count(int client_fd, uint32_t printable_count);

uint32_t pcc_total[95];
int client_socket_fd = -1;
int is_server_running = 1;

void handle_SIGINT(int signum) {
    if (client_socket_fd == -1) {
        display_statistics();
    }
    is_server_running = 0;
}
/* dispaly statistics of printable characters */

void display_statistics() {
    for (int i = 0; i < 95; i++) {
        printf("char '%c' : %u times\n", (char)(i + 32), pcc_total[i]);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Usage: <server port>\n");
        exit(EXIT_FAILURE);
    }

    int server_socket_fd;
    setup_server(&server_socket_fd, argv[1]);

    
    struct sigaction sigint_action = {
        .sa_handler = handle_SIGINT,
        .sa_flags = SA_RESTART
    };
    if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
        perror("Signal handler registration failed");
        exit(EXIT_FAILURE);
    }
    /* Main server loop to handle client connections */

    while (is_server_running) {
            client_socket_fd = accept(server_socket_fd, NULL, NULL);
        if (client_socket_fd < 0) {
            perror("Accept failed");
            continue;
        }

        handle_client_connection(client_socket_fd);

        close(client_socket_fd);
        client_socket_fd = -1;
    }

    display_statistics();
}

void setup_server(int *server_fd, const char *port) {
    int enable = 1;
    struct sockaddr_in server_address;

    
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(port));

    
    if (bind(*server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

        if (listen(*server_fd, 10) != 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

   /* Initiazling to 0*/
    memset(pcc_total, 0, sizeof(pcc_total));
}

void handle_client_connection(int client_fd) {
    uint32_t temp_pcc_total[95];
    uint32_t file_size, printable_count = 0;
    memset(temp_pcc_total, 0, sizeof(temp_pcc_total));

     /* Receive and process data from client */
    receive_data(client_fd, &file_size, &printable_count, temp_pcc_total);
    
    send_printable_count(client_fd, printable_count);

    for (int i = 0; i < 95; i++) {
        pcc_total[i] += temp_pcc_total[i];
    }
}

void receive_data(int client_fd, uint32_t *file_size, uint32_t *printable_count, uint32_t temp_pcc_total[95]) {
    int total_received = 0, remaining = 4, cur_received;
    char buffer[1000000]; 

    /* Receive the file size from client */
    while (remaining > 0) {
        cur_received = read(client_fd, (char *)file_size + total_received, remaining);
        if (cur_received == 0 || (cur_received < 0 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
            perror("Failed to receive file_size from the client");
            close(client_fd);
            return;
        } else if (cur_received < 0) {
            perror("Error receiving file_size from client");
            close(client_fd);
            exit(EXIT_FAILURE);
        } else {
            total_received += cur_received;
            remaining -= cur_received;
        }
    }
    *file_size = ntohl(*file_size);
    /* Receive the file content from client */
    remaining = *file_size;
    while (remaining > 0) {
        int chunk_size = sizeof(buffer) < remaining ? sizeof(buffer) : remaining;
        cur_received = read(client_fd, buffer, chunk_size);
        if (cur_received <= 0) {
            perror("Failed to receive file content from the client");
            close(client_fd);
            return;
        }
        for (int i = 0; i < cur_received; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) {
                temp_pcc_total[(int)(buffer[i]) - 32]++;
                (*printable_count)++;
            }
        }
        remaining -= cur_received;
    }/* Convert count to network byte order */
    *printable_count = htonl(*printable_count);
}
void send_printable_count(int client_fd, uint32_t printable_count) {
    int total_sent = 0, cur_sent, remaining = 4;

    /* Send the printable character count to client */
    while (remaining > 0) {
        cur_sent = write(client_fd, (char *)&printable_count + total_sent, remaining);
        if (cur_sent == 0 || (cur_sent < 0 && (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE))) {
            perror("Failed to send printable character count to the client");
            close(client_fd);
            return;
        } else if (cur_sent < 0) {
            perror("Error sending printable character count to client");
            close(client_fd);
            exit(EXIT_FAILURE);
        } else {
            total_sent += cur_sent;
            remaining -= cur_sent;
        }
    }
}