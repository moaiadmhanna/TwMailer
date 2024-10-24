#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>  
#include <sys/types.h> 
#include <dirent.h>
#include <errno.h>
#define BUFFER_SIZE 1024 // Buffer size for communication
#define MAX_USER_CHAR 9 // Max username length (8 chars + null terminator)
// Structure to represent an email
typedef struct{
    char *sender;
    char *receiver;
    char *subject;
    char *message;
} Mail_Body;
// Function declarations
void create_socket(int *sfd);
void setup_socket(int sfd, struct sockaddr_in *serveraddr);
void trim(char *input);
void listening(int sfd);
int communication(int consfd, char *buffer, int buffersize,char* mail_dir, char* current_user);
void accept_client(int sfd, int *peersoc, char* buffer,int buffer_size ,char* mail_dir, int* connected);
void receive_message(int peersoc, char *buffer, int buflen, char* mail_dir);
void handle_send_client(char *buffer,int consfd, char* mail_dir, char* current_user);
void handle_list_message(char* buffer,int consfd, char* mail_dir, char* current_user);
void handle_del_message(char *buffer,int consfd, char* mail_dir, char* current_user);
void handle_read_message(char *buffer,int consfd, char* mail_dir, char* current_user);
void handle_quit_message(char *buffer, int consfd, char* mail_dir, char* current_user);
void send_client(char *message, int consfd);
void save_message(Mail_Body mail_body, int consfd, char* mail_dir);
char** list_message(char* receiver_path, char* sender);
int get_messages_count(char **messages);
int tokenize_message(char *buffer, char **send_obj, int times);
void read_file(FILE *file, int consfd);
char *get_path_of_index(char **send_obj, int consfd, char *mail_dir);
char *get_user_dir(char *mail_dir, char *username);
int check_buffer(int size, int consfd);
// Command options for the user (SEND, LIST, etc.)
typedef struct {
    char* name;
    void (*func)(char*, int, char*, char*);
} option;
// List of available user options
option options[] = {
    {"SEND", handle_send_client},
    {"LIST", handle_list_message},
    {"DEL", handle_del_message},
    {"READ", handle_read_message},
    {"QUIT", handle_quit_message}
};
int main(const int argc, char *argv[]){
    char* port = argv[1];
    char* mail_dir = argv[2];

    // Create mail directory if not exists
    DIR* dir = opendir(mail_dir);
    if(!dir)
        mkdir(mail_dir, 0777);

    int sfd;
    create_socket(&sfd);

    struct sockaddr_in serveraddr = {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(port)),
            .sin_addr.s_addr = INADDR_ANY
    };

    inet_aton("127.0.0.3", &serveraddr.sin_addr);
    setup_socket(sfd, &serveraddr);
    listening(sfd);

    int peersoc; // Socket of client
    char buffer[BUFFER_SIZE];
    int connected =0;

    // Main loop to accept and serve clients
    while(1)
        accept_client(sfd, &peersoc, buffer,BUFFER_SIZE, mail_dir, &connected); 
    close(sfd);
    return 0;
}
// Create a socket (TCP stream)
void create_socket(int *sfd){
    *sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sfd == -1) {
        printf("Socket creation failed with error %d . \n", errno);
        exit(EXIT_FAILURE);
    }
}
// Set socket options and bind to the port
void setup_socket(int sfd, struct sockaddr_in* serveraddr){
    int enable = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        printf("Unable to set socket options due to %d \n", errno);
        exit(EXIT_FAILURE);
    }
    if (bind(sfd, (struct sockaddr *)serveraddr, sizeof(*serveraddr)) == -1) {
        printf("Binding failed with error %d ", errno);
        exit(EXIT_FAILURE);
    }
}
// Start listening for incoming connections
void listening(int sfd){
    int constat = listen(sfd, 6);
    if (constat == -1) {
        printf("Connection could not be established. Socket is unable to accept new connections.\n %d", errno);
        exit(EXIT_FAILURE);
    }
}
// Check if the Buffer return -1 
int check_buffer(int size, int consfd){
    if(size == -1) {
        exit(EXIT_FAILURE);
    }
    return size;
}
// Accept a client connection and ask for a username
void accept_client(int sfd, int *peersoc, char* buffer,int buffer_size, char* mail_dir, int* connected){
    memset(buffer, 0, buffer_size);
    struct sockaddr client;
    socklen_t addrlen = sizeof(client);

    // Accept client if no one is connected
    if(!*connected){
        if ((*peersoc = accept(sfd, &client, &addrlen)) == -1) {
            printf("Unable to accept client connection");
            exit(EXIT_FAILURE);
        }
        *connected = 1;
    }
    
    send_client("What is ur Username: ", *peersoc);
    check_buffer(recv(*peersoc, &buffer[0], buffer_size, 0), *peersoc);

    // Validate username length
    if(strlen(buffer) > MAX_USER_CHAR){
        send_client("Username should have max 8 char.", *peersoc);
        return;
    }
    send_client("OK", *peersoc);
    printf("Accepted with filedescriptor of requesting socket %d \n", *peersoc);
    receive_message(*peersoc,buffer,BUFFER_SIZE, mail_dir); 
}
// Handle communication for the connected user (until "QUIT")
void receive_message(int peersoc, char* buffer, int buflen, char* mail_dir){
    char *current_user = malloc(strlen(buffer) + 1 + 1);
    strcpy(current_user, buffer);
    trim(current_user);

    // Communication loop until user quits
    while(communication(peersoc,buffer,buflen, mail_dir, current_user) == 0);
}
// Main communication logic: present options and handle user commands
int communication(int consfd, char *buffer, int buffersize, char* mail_dir, char* current_user)
{
    memset(buffer, 0, buffersize);
    send_client("Option (SEND | DEL | READ | LIST | QUIT)", consfd);
    check_buffer(recv(consfd, &buffer[0], buffersize, 0), consfd);
    char option[BUFFER_SIZE];
    strcpy(option,buffer);
    trim(option);

    // Match user command and call the corresponding function
    for(int i = 0; i < sizeof(options)/sizeof(options[0]); i++){
        if(strcasecmp(option, options[i].name) == 0){
            options[i].func(buffer, consfd, mail_dir, current_user);
            return 0;
        }
    }
    send_client("Invalid option", consfd);
    return 0;
}
// Function to get the User directory using the maildir and the Username
char* get_user_dir(char* mail_dir, char* username){
    char *mail_path = malloc(strlen(mail_dir) + strlen(username) + 2);
    sprintf(mail_path, "%s/%s", mail_dir, username);
    return mail_path;
}
// Trim trailing newline or spaces from input
void trim(char *input)
{
    if(input == NULL) return;
    int len = strlen(input);
    if (input[len - 1] == '\r' || input[len - 1] == '\n' || input[len - 1] == ' ') {
        input[len - 1] = '\0';
    }
    if (input[len - 2] == '\r' || input[len - 2] == '\n' || input[len - 2] == ' ' ) {
        input[len - 2] = '\0';
    }
}
// Function to tokenize the message and save it in the send_obj array
int tokenize_message(char* buffer, char** send_obj, int times) {
    if(buffer[0]== '\n'){
        return 1;
    }

    int buffer_length = strlen(buffer);
    char *copied_buffer = malloc(buffer_length + 2);
    strcpy(copied_buffer, buffer);

    char *tokens = strtok(copied_buffer, "\n");
    trim(tokens);
    if (tokens == NULL){
        return 1;
    }
    // trim and tokenize the message according to \n
    for (int i = 0; i < times && tokens != NULL; i++) {
        trim(tokens);
        send_obj[i] = tokens;
        tokens = strtok(NULL, "\n");
    }
    return 0;
}
// Function to Handle "SEND" command: receive and save a message from the user
void handle_send_client(char* buffer, int consfd, char* mail_dir, char* current_user)
{
    // Receive Message
    int total = 0;
    int size = 0;
    while((size = check_buffer(recv(consfd, &buffer[total],BUFFER_SIZE,0), consfd)) > 0) {
        total += size;
        if (buffer[total - size] == '.' && size == 2) {
            break; 
        }
    };
    int body_length = 4;
    char *send_obj[4]  = {'\0'};
    // Handle the first user inputs (Sender,Receiver,Subject).
    if(tokenize_message(buffer, send_obj, body_length -1) == 1){
        send_client("ERR", consfd);
        return;
    }
    for(int i = 0; i < body_length -1; i++)
    {
        if(send_obj[i] == NULL)
        {
            send_client("ERR", consfd);
            return;
        }
    }
    // Handle the message from the User
    char *tokens = strtok(buffer, "\n");
    for (int i = 0; i < body_length - 2 && tokens != NULL; i++) {
        tokens = strtok(NULL, "\n");  // to skip the first 3 user inputs
    }
    char *full_message = strtok(NULL, "");
    if (full_message == NULL) {
        send_client("ERR", consfd);
        return;
    }

    // Remove the point at the End of the String
    size_t len = strlen(full_message);
    if (len > 0 && (full_message[len - 2] == '.')) {
        full_message[len - 2] = '\0';  
    }
    // Save the full Send message as mail_body struct
    Mail_Body mail_body = {
        sender: send_obj[0],
        receiver: send_obj[1],
        subject: send_obj[2],
        message: full_message
    };
    // Check if the sender is the logged user
    if(strcasecmp(mail_body.sender, current_user) != 0 || strlen(mail_body.receiver) > MAX_USER_CHAR){
        
        send_client("ERR", consfd);
        return;
    }
    save_message(mail_body, consfd, mail_dir);
    send_client("OK", consfd);
}
// Function to send the message to the corresponding socket
void send_client(char* message, int consfd) {
    int message_length = strlen(message);
    char *new = malloc(message_length + 2);
    strcpy(new, message);
    new[message_length + 1] = '\n';
    new[message_length + 2] = '\0';
     // Send the length of the message
    int new_length = strlen(new);
    send(consfd, &new_length, sizeof(new_length), 0); // to determine the length of the message 
    send(consfd, new, strlen(message), 0); // the message it self
    free(new);
}
// Function to quit and close the corresponding socket
void handle_quit_message(char* buffer, int consfd, char* mail_dir, char* current_user){
    printf("End connection with client. \n");
    close(consfd);
    exit(0);
}
// Function to count the number of messages 
int get_messages_count(char** messages){
    int count = 0;
    while (messages[count] != NULL) {
        count++;
    }
    return count;
}
// Function to get the Fullpath to save the message in (Root,Username directory)
char* get_full_path(char* root, char* fileName){
    char *path = malloc(strlen(root) + strlen(fileName) + 4); // +4 for slashes and null terminator
    sprintf(path, "%s/%s", root, fileName);
    return path;
}
// Function to Handle "list" command: receive and return the messages from the user directory
void handle_list_message(char* buffer, int consfd, char* mail_dir, char* current_user)
{
    check_buffer(recv(consfd, &buffer[0],BUFFER_SIZE,0), consfd);

    char* mail_path = get_user_dir(mail_dir, current_user);
    char **messages = list_message(mail_path, buffer);
    int count = 0;
    if (messages != NULL) {
        count = get_messages_count(messages);
    }
    // Check the number of messages in the User directory for the given username and send it
    char *number_of_messages = malloc(sizeof(int) * (count > 0 ? count:1) + strlen("Messages"));
    sprintf(number_of_messages, "%d Messages", count);
    send_client(number_of_messages, consfd);
    free(number_of_messages);
     if (messages != NULL) 
        {
            int total_len = 0;
            for (int i = 0; messages[i] != NULL; i++) {
                total_len += snprintf(NULL, 0, "%d: %s\n", i + 1, messages[i]);  // Add +1 for the null terminator
            }
            // Allocate memory for the long_message
            char *long_message = malloc(total_len + 2);
            if (long_message == NULL) {
                perror("Failed to allocate memory for long_message");
                exit(EXIT_FAILURE);
            }
            // Concatenate each message into long_message
            long_message[0] = '\0';  // Start with an empty string
            for (int i = 0; messages[i] != NULL; i++) {
                char *message = malloc(snprintf(NULL, 0, "%d: %s\n", i + 1, messages[i]) + 1);
                snprintf(message, snprintf(NULL, 0, "%d: %s\n", i + 1, messages[i]) + 1, "%d: %s\n", i + 1, messages[i]);
                strcat(long_message, message);
                free(message);  // Free the individual message after concatenating
                free(messages[i]);  // Free the original message
            }
            // Send the entire long_message to the client
            send_client(long_message, consfd);
            free(long_message);
        }
    // free(receiver_path);
    free(messages);

}
// Function save all messages in char array and return it 
char** list_message(char* receiver_path, char* sender) {
    trim(sender);
    char** messages = NULL;
    struct dirent *dir;
    int cnt = 0;
    DIR *d = opendir(receiver_path);
    if (d) {
        while ((dir = readdir(d)) != NULL)
        {
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
            if(strcasecmp(sender, "All") != 0){
                char* file_name_copy = strdup(dir->d_name);
                char* sender_name = strtok(file_name_copy, "-");
                if(strcasecmp(sender_name, sender) != 0)
                    continue;
            }
            int path_len = strlen(receiver_path) + strlen(dir->d_name) + 2; // +1 for '/', +1 for null terminator
            char *full_path = malloc(path_len);  // Adjust size as needed
            snprintf(full_path, path_len, "%s/%s", receiver_path, dir->d_name);
            struct stat file_stat;
            if (stat(full_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
                char **temp = realloc(messages, (cnt + 2) * sizeof(char*));
                messages = temp;
                messages[cnt] = strdup(dir->d_name);
                cnt++;
            }
        }
        closedir(d);
    }
    printf("%d", cnt);
    if(messages)
        messages[cnt] = NULL;
    return messages;
}
// Function to Handle "del" command: receive and delete the messages from the user directory
void handle_del_message(char* buffer, int consfd, char* mail_dir, char* current_user){
    if (check_buffer(recv(consfd, buffer, BUFFER_SIZE, 0), consfd) == -1 || check_buffer(recv(consfd, buffer + strlen(buffer), BUFFER_SIZE, 0), consfd) == -1) {
        printf("cannot receive due to %d \n", errno);
        exit(EXIT_FAILURE);
    }

    char *send_obj[2]  = {'\0'};
    if(tokenize_message(buffer, send_obj, 2) == 1){
        send_client("ERR", consfd);
        return;
    }
    char* mail_path = get_user_dir(mail_dir, current_user);
    char* path_of_index = get_path_of_index(send_obj, consfd, mail_path);
    if(path_of_index == NULL || remove(path_of_index) != 0){
        send_client("ERR", consfd);
    }
    else{
        send_client("OK", consfd);
        char **messages = list_message(mail_dir, send_obj[0]);
        if(messages){
            printf("Nice\n");
        }
        if (!messages) remove(mail_dir);
    }

    free(path_of_index);
}
// Function to Handle "read" command: receive and sending the messages from the user directory
void handle_read_message(char* buffer, int consfd, char* mail_dir, char* current_user){
    memset(buffer, 0, BUFFER_SIZE);
    int size = check_buffer(recv(consfd, &buffer[0],BUFFER_SIZE,0), consfd);
    check_buffer(recv(consfd, &buffer[size],BUFFER_SIZE,0), consfd);
    char *send_obj[2]  = {'\0'};
    if(tokenize_message(buffer, send_obj, 2) == 1){
        send_client("ERR", consfd);
        return;
    }

    char* mail_path = get_user_dir(mail_dir, current_user);
    char* path_of_index = get_path_of_index(send_obj, consfd, mail_path);
    if(path_of_index == NULL){
        send_client("ERR", consfd);
        return;
    }
    FILE* file = fopen(path_of_index, "r");

    read_file(file, consfd);
    free(path_of_index);
}
// Function to get the path of a message according to the message index
char* get_path_of_index(char** send_obj, int consfd, char* mail_dir) {
    char **messages = list_message(mail_dir, send_obj[0]);
    int message_index = atoi(send_obj[1]) - 1;
    if (!messages || message_index < 0 || message_index >= get_messages_count(messages)) {
        if (messages) free(messages);
        return NULL;
    }

    char* full_path = get_full_path(mail_dir, messages[message_index]);
    
    for (int i = 0; messages[i]; i++) free(messages[i]);
    free(messages);
    return full_path;
}
// Function used to read and print out the file
void read_file(FILE* file, int consfd){

    char line[256];
    if (file != NULL) {
        char buffer[BUFFER_SIZE] = "";  // Buffer that saves all lines of the file
        while (fgets(line, sizeof(line), file) != NULL) {
            if (strlen(line) > 0) {
                strncat(buffer, line, sizeof(buffer) - strlen(buffer) - 1);  // check the size of buffer
            }
        }
        
        send_client(strlen(buffer) <= 0 ? "<Message is empty>":buffer, consfd);
        fclose(file);
    }
    else {
        send_client("Unable to open file!", consfd);
        exit(EXIT_FAILURE);
    }
}
// Function to save the message to a file in the user's mail directory
void save_message(Mail_Body mail_body, int consfd,  char* mail_dir) {
    FILE *fptr;
    int root_dir_len = strlen(mail_dir);
    int dir_len = strlen(mail_body.receiver);
    int path_sender_len = strlen(mail_body.sender);
    int path_sub_len = strlen(mail_body.subject);
    int type_len = strlen(".txt");
    int sep_len = strlen("-");

    // Allocate memory for the receiver directory name
    char *dirname = malloc(dir_len + 1);
    strcpy(dirname, mail_body.receiver);

    // Full path for the receiver directory
    char *receiver_path = malloc(root_dir_len + dir_len + 2); // +2 for slash and null terminator
    sprintf(receiver_path, "%s/%s", mail_dir, dirname);

    // Check if the receiver's directory exists, create it if not
    if (access(receiver_path, F_OK) == -1) {
        mkdir(receiver_path, 0777);
    }

    // Full file path
    char *path = malloc(root_dir_len + dir_len + path_sender_len + sep_len + path_sub_len + type_len + 20); // +4 for slashes and null terminator
    sprintf(path, "%s/%s/%s-%s.txt", mail_dir, dirname, mail_body.sender, mail_body.subject);

    // When same file exists
    int counter = 1;
    while (access(path, F_OK) != -1) { 
        sprintf(path, "%s/%s/%s-%s(%d).txt", mail_dir, dirname, mail_body.sender, mail_body.subject, counter);
        counter++;
    }

    fptr = fopen(path, "w");

    // Write the message to the file
    fprintf(fptr, "%s", mail_body.message);

    // Close the file
    fclose(fptr);
    free(path);
    free(dirname);
    free(receiver_path);
}