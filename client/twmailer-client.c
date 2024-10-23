#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>


#define BUFFER_SIZE 1024

enum SENDING_STATE {Break = 1, Success = 0, Continue = 2};



void create_socket(int *sockfd);
void connect_to_server(int sockfd, struct sockaddr_in *serveraddr);
void send_to_server(int sockfd, const char *message);
void recv_from_server(int sockfd, char *buffer, int buffer_size);
void handle_server_communication(int sockfd);
enum SENDING_STATE list_messages(int sockfd, char* input, char* buffer);
enum SENDING_STATE read_message(int sockfd, char* input, char* buffer);
enum SENDING_STATE send_message(int sockfd, char* input, char* buffer);
enum SENDING_STATE delete_message(int sockfd, char* input, char* buffer);
enum SENDING_STATE quit_client(int sockfd, char *input, char *buffer);

typedef struct {
    char* name;
    enum SENDING_STATE (*func)(int, char*, char*);
} option;

option options[] = {
    {"SEND", send_message},
    {"LIST", list_messages},
    {"DEL", delete_message},
    {"READ", read_message},
    {"QUIT", quit_client}
};




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

    if (connect(sockfd, (struct sockaddr *)serveraddr, sizeof(*serveraddr)) == -1) {
        perror("Failed to connect to the server");
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
    int message_length;
    recv(sockfd, &message_length, sizeof(message_length), 0);
    int bytes_received = recv(sockfd, buffer, message_length, 0);
    if (bytes_received == -1) {
        perror("Failed to receive message from server");
        exit(EXIT_FAILURE);
    }
    printf("<< %s\n", buffer);
}

void input_client(char* output, char* input, int bufflen){
    printf("%s ", output);
    if(fgets(input, bufflen, stdin) == NULL){
        exit(EXIT_FAILURE);
    }
}

void prepare_and_send_messages(char** body, int sockfd, char* input){
     for(int b = 0; body[b] != NULL; b++){
        input_client(body[b], input, BUFFER_SIZE);
        send_to_server(sockfd, input);
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
        if(strcasecmp(buffer,"OK") == 0) break;
    }
    enum SENDING_STATE state = Success;
    do {
        // Receive option prompt
        // for options
        memset(input,0,BUFFER_SIZE);
        recv_from_server(sockfd, buffer, BUFFER_SIZE);

        input_client(">>", input, BUFFER_SIZE);
        input[strcspn(input, "\n")] = 0;  // Remove newline
        send_to_server(sockfd, input);

        for(int i = 0; i < sizeof(options)/sizeof(options[0]); i++){
            if(strcasecmp(input, options[i].name) == 0){
                state = options[i].func(sockfd, input, buffer);
            }
        }
        if(state == Continue) {
            state = Success;
            continue;
        }

        // Receive response from server
        recv_from_server(sockfd, buffer, BUFFER_SIZE);
    } while (state == Success);
}

enum SENDING_STATE list_messages(int sockfd, char* input, char* buffer){
    // List messages
    char *body[] = {"Sender name (or 'All') >>", NULL};
    prepare_and_send_messages(body, sockfd, input);

    // for the number of messages
    recv_from_server(sockfd, buffer, BUFFER_SIZE);
    if(atoi(strtok(buffer, " ")) == 0)
        return Continue;
    return Success;
}

enum SENDING_STATE read_message(int sockfd, char* input, char* buffer){
    char *body[] = {"Sender >>", "Message number >>", NULL};
    prepare_and_send_messages(body, sockfd, input);
    return Success;
}

enum SENDING_STATE delete_message(int sockfd, char* input, char* buffer){
    char *body[] = {"Sender >>", "Message number >>", NULL};
    prepare_and_send_messages(body, sockfd, input);
    return Success;
}

enum SENDING_STATE send_message(int sockfd, char* input, char* buffer){
    // Send a message
    char *body[] = {"Sender >>", "Receiver >>", "Subject >>", NULL};
    prepare_and_send_messages(body, sockfd, input);
    while(1)
    {
        input_client(">>", input, BUFFER_SIZE);
        send_to_server(sockfd, input);
        if(strcasecmp(input, ".\n") == 0) break;
    }
    return Success;
}

enum SENDING_STATE quit_client(int sockfd, char* input, char* buffer){
    send_to_server(sockfd, "QUIT");
    exit(0);
}