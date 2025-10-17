#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int port = 9999;
    
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    printf("Connecting to localhost:%d...\n", port);
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }
    printf("Connected!\n\n");
    
    printf("Sending: HELLO\n");
    send(sock, "HELLO\n", 6, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        printf("Received: %s\n", buffer);
    }
    
    printf("\nSending: LIST-REFS\n");
    send(sock, "LIST-REFS\n", 10, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        printf("Received: %s\n", buffer);
    }
    
    printf("\nSending: QUIT\n");
    send(sock, "QUIT\n", 5, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        printf("Received: %s\n", buffer);
    }
    
    close(sock);
    printf("\nConnection closed.\n");
    
    return 0;
}
