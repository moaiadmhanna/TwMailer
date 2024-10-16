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
void handle_send_message(char *send_obj[4]);

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
        if (total >= 3 && buffer[total - 3] == '.') {
            break; 
        }
    };
    if(size == -1) {
        printf("cannot receive due to %d \n", errno);
        exit(EXIT_FAILURE);
    }
    send(consfd, "OK\n", 3, 0);

    char *tokens = strtok(buffer, "\n");
    trim(tokens);
    // printf("Token received [%s] with size %d \n", tokens, (int)strlen(tokens));
    if (tokens == NULL) {
        exit(EXIT_FAILURE);
    } 
    else if (strcmp(tokens, "SEND") == 0) 
    {
        printf("---------- SEND ----------- \n");
        char *send_obj[4];
        for(int i = 0; i < 4; i++){
            if(tokens == NULL){
                break;
            }
            tokens = strtok(NULL, "\n"); 
            trim(tokens);
            printf("Reveiced %s \n", tokens);
            send_obj[i] = tokens;
        }
        for(int i = 0; i < 4; i++) {
            printf("%s", send_obj[i]);
        }
        // handle_send_message(send_obj);
    } 
    else if (strcmp(tokens, "START") == 0) 
    {
        printf("Received START\n");
    } 
    else if (strcmp(tokens, "END") == 0) 
    {
        printf("end connection with client. \n");
        close(consfd);
        return 1;
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

void receive_message(int peersoc, char* buffer, int buflen){
    buffer[0] = '\0';
    while(communication(peersoc,buffer,buflen) == 0);
}

void trim(char *input)
{
    if(input == NULL) return;
    int len = strlen(input);
    if (input[len - 1] == '\r' || input[len - 1] == '\n' || input[len - 1] == ' ' || input[len - 1] == '.' ) {
        input[len - 1] = '\0';
    }
    if (input[len - 2] == '\r' || input[len - 2] == '\n' || input[len - 2] == ' ' || input[len - 2] == '.' ) {
        input[len - 2] = '\0';
    }

}

void handle_send_message(char *send_obj[4]){
    
}