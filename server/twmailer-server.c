#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>  
#include <sys/types.h> 
#include <dirent.h>
#include <errno.h>
#define BUFFER_SIZE 50


typedef struct{
    char *sender;
    char *receiver;
    char *subject;
    char *message;
} Mail_Body;


void create_socket(int *sfd);
void setup_socket(int sfd, struct sockaddr_in *serveraddr);
void trim(char *input);
void listening(int sfd);
int communication(int consfd, char *buffer, int buffersize,char* mail_dir);
void accept_client(int sfd, int *peersoc);
void receive_message(int peersoc, char *buffer, int buflen, char* mail_dir);
void handle_send_client(char *buffer,int consfd, char* mail_dir);
void handle_list_message(char* current_user,int consfd, char* mail_dir);
void handle_del_message(char *buffer,int consfd, char* mail_dir);
void handle_read_message(char *buffer,int consfd, char* mail_dir);
void handle_quit_message(char *buffer, int consfd, char* mail_dir);
void send_client(char *message, int consfd);
void save_message(Mail_Body mail_body, int consfd, char* mail_dir);
char** list_message(char* receiver_path);

typedef struct {
    char* name;
    void (*func)(char*, int, char*);
} option;



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
    while(1)
    {
        accept_client(sfd, &peersoc); 
        receive_message(peersoc,buffer,BUFFER_SIZE, mail_dir); 
    }
    close(sfd); 
    return 0;
}

void create_socket(int *sfd){
    *sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sfd == -1) {
        printf("Socket creation failed with error %d . \n", errno);
        exit(EXIT_FAILURE);
    }
}

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

void listening(int sfd){
    int constat = listen(sfd, 6);
    if (constat == -1) {
        printf("Connection could not be established. Socket is unable to accept new connections.\n %d", errno);
        exit(EXIT_FAILURE);
    }
}

int communication(int consfd, char *buffer, int buffersize, char* mail_dir)
{
    memset(buffer, 0, buffersize);
    int size = recv(consfd, &buffer[0],buffersize,0);
    if(size == -1) {
        printf("cannot receive due to %d \n", errno);
        exit(EXIT_FAILURE);
    }

    char option[BUFFER_SIZE];
    strcpy(option,buffer);
    trim(option);
    for(int i = 0; i < sizeof(options)/sizeof(options[0]); i++){
        if(strcasecmp(option, options[i].name) == 0){
            options[i].func(buffer, consfd, mail_dir);
        }
    }
    return 0;
}

void accept_client(int sfd, int* peersoc){
    struct sockaddr client;
    socklen_t addrlen = sizeof(client);
    if ((*peersoc = accept(sfd, &client, &addrlen)) == -1) {
        printf("Unable to accept client connection");
        exit(EXIT_FAILURE);
    }

    printf("Accepted with filedescriptor of requesting socket %d \n", *peersoc);
}

void receive_message(int peersoc, char* buffer, int buflen, char* mail_dir){
    buffer[0] = '\0';
    while(communication(peersoc,buffer,buflen, mail_dir) == 0);
}

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

void tokenize_message(char* buffer, char** send_obj) {
    char *tokens = strtok(buffer, "\n");
    trim(tokens);
    if (tokens == NULL)
        exit(EXIT_FAILURE);
    for (int i = 0; i < 5 && tokens != NULL; i++) {
        trim(tokens);
        send_obj[i] = tokens;
        tokens = strtok(NULL, "\n");
    }
}

void handle_send_client(char* buffer, int consfd, char* mail_dir)
{
    // Receive Message
    int total = 0; 
    int size = 0;
    while((size = recv(consfd, &buffer[total],BUFFER_SIZE,0)) > 0) {
        total += size;
        if (buffer[total - size] == '.' && size == 3) {
            break; 
        }
    };

    // Tokenize Message
    char *send_obj[5]  = {'\0'};
    tokenize_message(buffer, send_obj);
   
    for(int i = 0; send_obj[i] != NULL; i++)
    {
        if(send_obj[i] == NULL)
        {
            send_client("Invalid Argument", consfd);
            return;
        }
    }
    Mail_Body mail_body = {
        sender: send_obj[0],
        receiver: send_obj[1],
        subject: send_obj[2],
        message: send_obj[3]
    };
    save_message(mail_body, consfd, mail_dir);
    send_client("Message sended!", consfd);
}

void send_client(char* message, int consfd) {
    int message_length = strlen(message);
    char *new = malloc(message_length + 1 + 1);

    strcpy(new, message);
    new[message_length] = '\n';
    new[message_length + 1] = '\0';

    send(consfd, new, strlen(new), 0);
    free(new);
}

void handle_quit_message(char* buffer, int consfd, char* mail_dir){
    printf("End connection with client. \n");
    close(consfd);
    exit(0);
}

void handle_list_message(char* current_user, int consfd, char* mail_dir)
{
    int size = recv(consfd, &current_user[0],BUFFER_SIZE,0);
    if(size == -1) {
        printf("cannot receive due to %d \n", errno);
        exit(EXIT_FAILURE);
    }
    trim(current_user);
    char *receiver_path = malloc(strlen(mail_dir) + strlen(current_user) + 2); // +2 for slash and null terminator
    sprintf(receiver_path, "%s/%s", mail_dir, current_user);
    
    char **messages = list_message(receiver_path);
    int count = 0;
    if (messages != NULL) {
        while (messages[count] != NULL) {
            count++;
        }
    }

    char *number_of_messages = malloc(sizeof(int) * (count > 0 ? count:1) + strlen("Messages"));
    sprintf(number_of_messages, "%d Messages", count);
    send_client(number_of_messages, consfd);
    free(number_of_messages);


     if (messages != NULL) {
        for (int i = 0; messages[i] != NULL; i++) {
            int message_len = snprintf(NULL, 0, "%d: %s", i + 1, messages[i]) + 1;
            char *message = malloc(message_len);
            snprintf(message, message_len, "%d: %s", i + 1, messages[i]);
            send_client(message, consfd);
            free(message);
        }
    }

    free(receiver_path);
    free(messages);

}

char** list_message(char* receiver_path) {
    char** messages = NULL;
    struct dirent *dir;
    DIR *d = opendir(receiver_path);
    if (d) {
        for (int cnt = 0; (dir = readdir(d)) != NULL; )
        {
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
            int path_len = strlen(receiver_path) + strlen(dir->d_name) + 2; // +1 for '/', +1 for null terminator
            char *full_path = malloc(path_len);  // Adjust size as needed
            snprintf(full_path, path_len, "%s/%s", receiver_path, dir->d_name);
            struct stat file_stat;
            if (stat(full_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
                char **temp = realloc(messages, (cnt + 1) * sizeof(char*));
                messages = temp;
                messages[cnt] = strdup(dir->d_name);
                cnt++;
            }
        }
        closedir(d);
    }
    return messages;
}

void handle_del_message(char* buffer, int consfd, char* mail_dir){
    printf("Del messages \n");

}

void handle_read_message(char* buffer, int consfd, char* mail_dir){
    printf("Read messages \n");

}

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
