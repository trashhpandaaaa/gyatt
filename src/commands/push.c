#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../gyatt.h"

#define BUFFER_SIZE 4096

int parse_remote_url(const char *url, char *hostname, int *port) {
    char *colon = strchr(url, ':');
    if (colon) {
        size_t len = colon - url;
        if (len >= 256) return -1;
        memcpy(hostname, url, len);
        hostname[len] = '\0';
        *port = atoi(colon + 1);
    } else {
        strncpy(hostname, url, 255);
        hostname[255] = '\0';
        *port = 9418;
    }
    return 0;
}

int connect_to_server(const char *hostname, int port) {
    int sock;
    struct sockaddr_in server_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, hostname, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Invalid address\n");
        close(sock);
        return -1;
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }
    
    return sock;
}

int send_command(int sock, const char *cmd, char *response, size_t response_size) {
    send(sock, cmd, strlen(cmd), 0);
    usleep(50000);
    
    memset(response, 0, response_size);
    int bytes = recv(sock, response, response_size - 1, 0);
    return bytes;
}

int cmd_push(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: gyatt push <remote> [branch]\n");
        fprintf(stderr, "Example: gyatt push 127.0.0.1:9999 main\n");
        return 1;
    }
    
    const char *remote_url = argv[1];
    
    char hostname[256];
    int port;
    if (parse_remote_url(remote_url, hostname, &port) != 0) {
        fprintf(stderr, "Error: Invalid remote URL\n");
        return 1;
    }
    
    printf("Connecting to %s:%d...\n", hostname, port);
    int sock = connect_to_server(hostname, port);
    if (sock < 0) {
        return 1;
    }
    
    printf("✓ Connected to remote server\n");
    
    char response[BUFFER_SIZE];
    if (send_command(sock, "HELLO\n", response, sizeof(response)) < 0) {
        fprintf(stderr, "Error: Handshake failed\n");
        close(sock);
        return 1;
    }
    
    printf("✓ Handshake successful\n");
    printf("\nPush functionality will transfer commits and objects to remote.\n");
    printf("Full implementation coming soon!\n");
    
    send_command(sock, "QUIT\n", response, sizeof(response));
    close(sock);
    
    return 0;
}
