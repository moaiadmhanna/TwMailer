#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void create_socket(int *sfd);
void setup_socket(int sfd, struct sockaddr_in *serveraddr);
void trim(char *input);
void listening(int sfd);
int communication(int consfd, char *buffer, int buffersize);
void accept_client(int sfd, int *peersoc);
void receive_message(int peersoc, char *buffer, int buflen);
void handle_options(char *buffer, int consfd);
void handle_send_message(char* tokens,int consfd);
void handle_list_message(char* tokens,int consfd);
void handle_del_message(char* tokens,int consfd);
void handle_read_message(char* tokens,int consfd);
void handle_quit_message(char* tokens, int consfd);

typedef struct {
    char* name;
    void (*func)(char*, int);
} option;

option options[] = {
    {"SEND", handle_send_message},
    {"LIST", handle_list_message},
    {"DEL", handle_del_message},
    {"READ", handle_read_message},
    {"QUIT", handle_quit_message}
};

int main(const int argc, char *argv[]){
    char* ip = argv[1];
    char* port = argv[2];

    int sfd;
    create_socket(&sfd);

    struct sockaddr_in serveraddr = {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(port)),
            .sin_addr.s_addr = INADDR_ANY
    };

    inet_aton(ip, &serveraddr.sin_addr);
    setup_socket(sfd, &serveraddr);
    listening(sfd);

    int peersoc; // Socket of client
    int buflen = 50;
    char buffer[buflen];
    while(1)
    {
        accept_client(sfd, &peersoc); 
        receive_message(peersoc,buffer,buflen); 
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
int communication(int consfd, char *buffer, int buffersize)
{
    memset(buffer, 0, buffersize);
    int size, total = 0;
    while((size = recv(consfd, &buffer[total],buffersize,0)) > 0) {
        // printf("Received iteration %s with size %d \n", &buffer[total], size);
        total += size;
        if (buffer[total - size] == '.' && size == 3) {
            break; 
        }
    };
    if(size == -1) {
        printf("cannot receive due to %d \n", errno);
        exit(EXIT_FAILURE);
    }
    send(consfd, "OK\n", 3, 0);
    handle_options(buffer,consfd);
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

void receive_message(int peersoc, char* buffer, int buflen){
    buffer[0] = '\0';
    while(communication(peersoc,buffer,buflen) == 0);
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

void handle_options(char *buffer,int consfd)
{
    char *tokens = strtok(buffer, "\n");
    trim(tokens);
    if (tokens == NULL)
    {
        exit(EXIT_FAILURE);
    } 
    for(int i = 0; i < sizeof(options)/sizeof(options[0]); i++){
        if(strcmp(tokens, options[i].name) == 0){
            options[i].func(tokens, consfd);
        }
    }
}
void handle_send_message(char* tokens, int consfd)
{
    printf("---------- SEND ----------- \n");
    char *send_obj[5]  = {'\0'};
    for(int i = 0; i < 5; i++){
        tokens = strtok(NULL, "\n"); 
        if(tokens == NULL){
            break;
        }
        trim(tokens);
        printf("Reveiced %s \n", tokens);
        if(strcmp(tokens,".") != 0)
            send_obj[i] = tokens;
    }
    for(int i = 0; i < 4; i++)
    {
        if(send_obj[i] == NULL)
        {
            printf("Message Object is not Complete\n");
            // return Message send Confirmation to the Client
            return;
        }
    }
    printf("Sender : %s \n", send_obj[0]);
    printf("Receiver : %s \n", send_obj[1]);
    printf("Subject : %s \n", send_obj[2]);
    printf("Message : %s \n", send_obj[3]);
    send(consfd,"OK Message Received\n",21,0);
}

void handle_quit_message(char* tokens, int consfd){
    printf("end connection with client. \n");
    close(consfd);
}

void handle_list_message(char* tokens, int consfd){
    printf("Hello list usernames \n");

}

void handle_del_message(char* tokens, int consfd){
    printf("Del messages \n");

}

void handle_read_message(char* tokens, int consfd){
    printf("Read messages \n");

}