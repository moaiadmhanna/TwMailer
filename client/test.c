#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1024

// Function to send data to the server
void send_data(int socket_fd, char *data) {
    if (send(socket_fd, data, strlen(data), 0) == -1) {
        perror("Send error");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Data: %s\n",data);
    }
}

// Function to receive data from the server
void receive_data(int socket_fd, char *buffer, int size) {
    int bytes_received = recv(socket_fd, buffer, size - 1, 0);
    printf("Bytes Received: %d \n",bytes_received);
    if (bytes_received == -1) {
        perror("Recv error");
        exit(EXIT_FAILURE);
    } else if(bytes_received == 0) {
        printf("No Response.\n");
        return;
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in server_address;
    char buffer[BUF_SIZE];
    char command[BUF_SIZE];
    int quit_flag = 0;

    // Check for the correct number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse the IP address and port
    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "Invalid port number. It must be between 1 and 65535.\n");
        return EXIT_FAILURE;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    // Set up the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    // Convert IP address from string to binary
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection error");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Connected to server %s:%d\n", server_ip, server_port);

    // Receive initial response from the server
    receive_data(sockfd, buffer, BUF_SIZE);
    printf("<< %s\n", buffer);

    while (!quit_flag) {
        memset(buffer, 0, BUF_SIZE);
        memset(command,0,BUF_SIZE);
        printf(">> ");  // Prompt to enter next command

        if (fgets(command, BUF_SIZE, stdin) != NULL) {
            // Remove newline characters from the command
            size_t len = strlen(command);
            if (len > 0 && command[len - 1] == '\n') {
                command[len - 1] = '\0';
            }

            // Debugging: Check what is entered
            printf("You entered: %s\n", command);
            printf("Buffer %s\n",buffer);

            // Handle QUIT command
            if (strcasecmp(command, "QUIT") == 0) {
                quit_flag = 1;
                send_data(sockfd, "QUIT\n");
                printf("Disconnecting from the server...\n");
                break;
            }
            else if(strcasecmp(command,"SEND") == 0)
            {
                send_data(sockfd,"SEND\n");
                printf("Sending data...\n");
                fgets(command,BUF_SIZE,stdin);
                fgets(command,BUF_SIZE,stdin);
                fgets(command,BUF_SIZE,stdin);
                fgets(command,BUF_SIZE,stdin);
                fgets(command,BUF_SIZE,stdin);
                send_data(sockfd, command);
            }

            // Receive server response (blocking call)
            printf("Break1\n");
            receive_data(sockfd, buffer, BUF_SIZE);
            if(strlen(buffer) > 0)
                printf("<< %s\n", buffer);
            printf("Break2\n");
        

            // Handle responses and potential server errors

        } else {
            printf("Error reading input\n");
            break;
        }
    }

    // Clean up and close the socket
    if (shutdown(sockfd, SHUT_RDWR) == -1) {
        perror("Shutdown error");
    }
    if (close(sockfd) == -1) {
        perror("Close error");
    }

    return EXIT_SUCCESS;
}