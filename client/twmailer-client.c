#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>

#define BUFFER_SIZE 1024

// Enumeration for message handling state
enum SENDING_STATE {Break = 1, Success = 0, Continue = 2};

// Function declarations
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

// Struct for menu options
typedef struct {
    char* name;
    enum SENDING_STATE (*func)(int, char*, char*);
} option;

// Menu options
option options[] = {
    {"SEND", send_message},
    {"LIST", list_messages},
    {"DEL", delete_message},
    {"READ", read_message},
    {"QUIT", quit_client}
};

int main(int argc, char *argv[]) {
    // Check if the user provided server IP and port
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    // Create socket
    int sockfd;
    create_socket(&sockfd);

    // Set up server address
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &serveraddr.sin_addr);

    // Connect to the server
    connect_to_server(sockfd, &serveraddr);

    // Handle communication with the server (main loop)
    handle_server_communication(sockfd);

    // Close socket before exiting
    close(sockfd);
    return 0;
}

// Function to create a socket
void create_socket(int *sockfd) {
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
}

// Function to connect to the server
void connect_to_server(int sockfd, struct sockaddr_in *serveraddr) {
    if (connect(sockfd, (struct sockaddr *)serveraddr, sizeof(*serveraddr)) == -1) {
        perror("Failed to connect to the server");
        exit(EXIT_FAILURE);
    }
}

// Function to send a message to the server
void send_to_server(int sockfd, const char *message) {
    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("Failed to send message to the server");
        exit(EXIT_FAILURE);
    }
}

// Function to receive a message from the server
void recv_from_server(int sockfd, char *buffer, int buffer_size) {
    memset(buffer, 0, buffer_size);
    int message_length;
    recv(sockfd, &message_length, sizeof(message_length), 0);
    int bytes_received = recv(sockfd, buffer, message_length, 0);
    if (bytes_received == -1) {
        perror("Failed to receive message from server");
        exit(EXIT_FAILURE);
    }
    printf("<< %s\n", buffer);  // Print received message
}

// Helper function to get input from the client and store it in 'input'
void input_client(char* output, char* input, int bufflen){
    printf("%s ", output);  // Prompt user
    if(fgets(input, bufflen, stdin) == NULL){
        exit(EXIT_FAILURE);
    }
}

// Prepare and send multiple messages as input from client
void prepare_and_send_messages(char** body, int sockfd, char* input){
    for(int b = 0; body[b] != NULL; b++){
        input_client(body[b], input, BUFFER_SIZE);  // Get user input
        send_to_server(sockfd, input);  // Send each part of the message
    }
}

// Main function to handle server communication and user options
void handle_server_communication(int sockfd) {
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    memset(input, 0, BUFFER_SIZE);

    // Initial server prompt (e.g., asking for username)
    while(1) {
        recv_from_server(sockfd, buffer, BUFFER_SIZE);  // Receive prompt
        input_client(">>", input, BUFFER_SIZE);  // Get user input (username)
        send_to_server(sockfd, input);  // Send to server

        // Check for server's OK response
        recv_from_server(sockfd, buffer, BUFFER_SIZE);
        if(strcasecmp(buffer,"OK") == 0) break;
    }

    enum SENDING_STATE state = Success;
    do {
        // Get and send the user's choice (SEND, LIST, READ, etc.)
        memset(input, 0, BUFFER_SIZE);
        recv_from_server(sockfd, buffer, BUFFER_SIZE);  // Receive menu prompt

        input_client(">>", input, BUFFER_SIZE);  // Get user option
        input[strcspn(input, "\n")] = 0;  // Remove newline from input
        send_to_server(sockfd, input);  // Send option to server

        // Match user input to a function and execute it
        for(int i = 0; i < sizeof(options)/sizeof(options[0]); i++){
            if(strcasecmp(input, options[i].name) == 0){
                state = options[i].func(sockfd, input, buffer);  // Call the function for the selected option
            }
        }

        if(state == Continue) {
            state = Success;
            continue;
        }

        // Receive server's response
        recv_from_server(sockfd, buffer, BUFFER_SIZE);
    } while (state == Success);  // Continue while state is Success
}

// Function to list messages
enum SENDING_STATE list_messages(int sockfd, char* input, char* buffer) {
    char *body[] = {"Sender name (or 'All') >>", NULL};  // Prompt user
    prepare_and_send_messages(body, sockfd, input);  // Get input and send

    recv_from_server(sockfd, buffer, BUFFER_SIZE);  // Receive response
    if(atoi(strtok(buffer, " ")) == 0)
        return Continue;  // If no messages, continue
    return Success;
}

// Function to read a specific message
enum SENDING_STATE read_message(int sockfd, char* input, char* buffer) {
    char *body[] = {"Sender >>", "Message number >>", NULL};  // Prompt for sender and message number
    prepare_and_send_messages(body, sockfd, input);  // Get input and send
    return Success;
}

// Function to delete a specific message
enum SENDING_STATE delete_message(int sockfd, char* input, char* buffer) {
    char *body[] = {"Sender >>", "Message number >>", NULL};  // Prompt for sender and message number
    prepare_and_send_messages(body, sockfd, input);  // Get input and send
    return Success;
}

// Function to send a message
enum SENDING_STATE send_message(int sockfd, char* input, char* buffer) {
    char *body[] = {"Sender >>", "Receiver >>", "Subject >>", "Message >>", NULL};  // Prompt for message details
    prepare_and_send_messages(body, sockfd, input);  // Get input and send

    // Continue sending message body until user inputs "."
    while(1) {
        input_client("Message (. to SEND) >>", input, BUFFER_SIZE);  // Message body
        send_to_server(sockfd, input);  // Send input
        if(strcasecmp(input, ".\n") == 0) break;  // End when "." is entered
    }
    return Success;
}

// Function to quit the client
enum SENDING_STATE quit_client(int sockfd, char* input, char* buffer) {
    send_to_server(sockfd, "QUIT");  // Send quit message to server
    exit(0);  // Exit the program
}
