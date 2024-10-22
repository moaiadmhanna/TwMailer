#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>


#define BUFFER_SIZE 1024

void create_socket(int *sockfd);
void connect_to_server(int sockfd, struct sockaddr_in *serveraddr);
void send_to_server(int sockfd, const char *message);
void recv_from_server(int sockfd, char *buffer, int buffer_size);
void handle_server_communication(int sockfd);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    // Create socket
    int sockfd;
    create_socket(&sockfd);

    // Set up server address struct
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &serveraddr.sin_addr);

    // Connect to the server
    connect_to_server(sockfd, &serveraddr);

    // Handle communication with the server
    handle_server_communication(sockfd);

    // Close socket
    close(sockfd);
    return 0;
}

void create_socket(int *sockfd) {
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
}

void connect_to_server(int sockfd, struct sockaddr_in *serveraddr) {
    int enable = 0;
    if (connect(sockfd, (struct sockaddr *)serveraddr, sizeof(*serveraddr)) == -1) {
        perror("Failed to connect to the server");
        exit(EXIT_FAILURE);
    }
     if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,&enable, sizeof(int)) == -1) {
        printf("Unable to set TCP_NODELAY due to %d \n", errno);
        exit(EXIT_FAILURE);
    }
}

void send_to_server(int sockfd, const char *message) {
    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("Failed to send message to the server");
        exit(EXIT_FAILURE);
    }
}

void recv_from_server(int sockfd, char *buffer, int buffer_size) {
    memset(buffer, 0, buffer_size);
    int bytes_received = recv(sockfd, buffer, buffer_size - 1, 0);
    if (bytes_received == -1) {
        perror("Failed to receive message from server");
        exit(EXIT_FAILURE);
    }
    printf("bytes Recived : %d\n",bytes_received);
    // buffer[bytes_received + 1] = '\0';
    printf("<< %s\n", buffer);
}

void input_client(char* output, char* input, int bufflen){
    printf("%s ", output);
    if(fgets(input, bufflen, stdin) == NULL){
        exit(EXIT_FAILURE);
    }
}

void handle_server_communication(int sockfd) {
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    memset(input,0,BUFFER_SIZE);
    // Receive initial server prompt (asking for username)
    while(1)
    {
        recv_from_server(sockfd, buffer, BUFFER_SIZE);
        // Send username to server

        input_client(">>", input, BUFFER_SIZE);
        send_to_server(sockfd, input);

        // for ok from the accept function
        recv_from_server(sockfd, buffer, BUFFER_SIZE);
        if(strcasecmp(buffer,"ok") == 0){break;}
    }


    while (1) {
        // Receive option prompt
        // for options
        memset(input,0,BUFFER_SIZE);
        recv_from_server(sockfd, buffer, BUFFER_SIZE);

        input_client(">>", input, BUFFER_SIZE);
        input[strcspn(input, "\n")] = 0;  // Remove newline
        send_to_server(sockfd, input);

        if (strcasecmp(input, "SEND") == 0) {
            // Send a message
            input_client("Sender >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

            input_client("Receiver >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

            input_client("Subject >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

            input_client("Message >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

            input_client(">>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

        } else if (strcasecmp(input, "LIST") == 0) {
            // List messages
            input_client("Sender name (or 'All') >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);
            recv_from_server(sockfd, buffer, BUFFER_SIZE);
            char *number_of_messages = strtok(buffer," ");
            printf("Number of messages %s \n",number_of_messages);
            for(int x = 0; x < atoi(number_of_messages); x++)
            {
                recv_from_server(sockfd, buffer, BUFFER_SIZE);
            }
            continue;

        } else if (strcasecmp(input, "DEL") == 0) {
            input_client("Sender >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

            input_client("Message number >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);

        } else if (strcasecmp(input, "READ") == 0) {
            // Read a message
            input_client("Sender >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);
            // to save the number of messages : 
            recv_from_server(sockfd, buffer, BUFFER_SIZE);

            input_client("Message number >>", input, BUFFER_SIZE);
            send_to_server(sockfd, input);


        } else if (strcasecmp(input, "QUIT") == 0) {
            // Quit the session
            send_to_server(sockfd, "QUIT");
            break;

        } else {
            printf("Invalid option. Try again.\n");
        }

        // Receive response from server
        recv_from_server(sockfd, buffer, BUFFER_SIZE);
    }
}