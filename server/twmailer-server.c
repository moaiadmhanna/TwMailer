#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void create_socket();

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
        printf("Connection could not be established. Socket is unable to accept new connections. %d", errno);
        exit(EXIT_FAILURE);
    }
    printf("Server is listining");
}
int communication(int consfd, char *buffer, int buffersize)
{
    memset(buffer, 0, buffersize);
    int size, total = 0;
    while((size = recv(consfd,buffer,buffersize,0)) > 0) {
        printf("Received iteration %s with size %d \n", &buffer[total], size);
        total += size;
    };
    if(size == -1) {
        printf("cannot receive due to %d \n", errno);
        exit(EXIT_FAILURE);
    }
    printf("received %s \n", buffer);
    send(consfd, "OK\n", 3, 0);
    /* TODO:
        Call strtok to spilt up the received string (initial call);
     */
    // char *tokens = /* TODO call strtok here */
    // trim(tokens);
    // printf("Token received [%s] with size %d \n", tokens, (int)strlen(tokens));
    // if (tokens == NULL) {
    //     exit(EXIT_FAILURE);
    // } else if (strcmp(tokens, "VOTE") == 0) {
    //     printf("Received VOTE \n");
    //     /* TODO
    //         Call strtok for next split iteration
    //      */
    //     tokens = /* call strtok here*/
    //     trim(tokens);
    //     printf("Reveiced Item after Vote %s \n", tokens);
    // } else if (strcmp(tokens, "START") == 0) {
    //     printf("Received START\n");
    // } else if (strcmp(tokens, "END") == 0) {
    //     printf("end connection with client. \n");
    //     close(consfd);
    //     return 1;
    // }
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
    // int size = recv(peersoc, &buffer, buflen - 1, 0);
    // printf("Message received %s with length %d \n.", buffer, size);
    buffer[0] = '\0';
    while(communication(peersoc,buffer,buflen) == 0);
}

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