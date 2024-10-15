#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

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

void accept_client(int sfd, int* peersoc){
    struct sockaddr client;
    socklen_t addrlen = sizeof(client);
    if ((*peersoc = accept(sfd, &client, &addrlen)) == -1) {
        printf("Unable to accept client connection");
        exit(EXIT_FAILURE);
    }

    printf("Accepted with filedescriptor of requesting socket %d \n", *peersoc);
}

int receive_message(int peersoc){
    int buflen = 50;
    char buffer[buflen];
    int size = recv(peersoc, &buffer, buflen - 1, 0);
    printf("Message received %s with length %d \n.", buffer, size);
    return size;
}

void send_message(int peersoc, int* size){
    char *response = "OK \n";
    *size = send(peersoc, response, sizeof(response), 0);    
}

int main(const int argc, char *argv[]){
    char* ip = argv[1];
    // char* port = argv[2];

    int sfd;
    create_socket(&sfd);

    struct sockaddr_in serveraddr = {
            .sin_family = AF_INET,
            .sin_port = htons(8888),
            .sin_addr.s_addr = INADDR_ANY
    };

    inet_aton(ip, &serveraddr.sin_addr);
    setup_socket(sfd, &serveraddr);
    listening(sfd);

    int peersoc; // Socket of client
    accept_client(sfd, &peersoc);

    int size_of_message= receive_message(peersoc);
    send_message(peersoc, &size_of_message);

    close(sfd); 
}