// Gyatt Server - Your personal Git server without the GitHub drama
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "../gyatt.h"
#include "../utils.h"
#include "../object.h"
#include "../hash.h"

#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

#define DEFAULT_PORT 9418  // Same as Git, because why not?
#define BUFFER_SIZE 8192

static volatile int server_running = 1;

// Gracefully handle Ctrl+C because we're polite like that
static void signal_handler(int signum) {
    (void)signum;
    printf("\n👋 Shutting down server...\n");
    server_running = 0;
}

// Our super sophisticated protocol (v1.0)
#define CMD_HELLO       "HELLO"
#define CMD_LIST_REFS   "LIST-REFS"
#define CMD_GET_OBJECT  "GET-OBJECT"
#define CMD_PUT_OBJECT  "PUT-OBJECT"
#define CMD_QUIT        "QUIT"

// Handle client connection (and their existential questions)
static void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    
    // Be friendly!
    const char *welcome = "GYATT-SERVER 1.0\n";
    send(client_socket, welcome, strlen(welcome), 0);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read <= 0) {
            break;  // Connection closed or error
        }
        
        // Remove trailing newline
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        printf("Client: %s\n", buffer);
        
        // Parse command
        if (strncmp(buffer, CMD_HELLO, strlen(CMD_HELLO)) == 0) {
            const char *response = "OK HELLO\n";
            send(client_socket, response, strlen(response), 0);
            
        } else if (strncmp(buffer, CMD_LIST_REFS, strlen(CMD_LIST_REFS)) == 0) {
            // List all branches
            char *gyatt_dir = get_gyatt_dir();
            if (!gyatt_dir) {
                const char *err = "ERROR No repository\n";
                send(client_socket, err, strlen(err), 0);
                continue;
            }
            
            char refs_path[PATH_MAX];
            snprintf(refs_path, sizeof(refs_path), "%s/refs/heads", gyatt_dir);
            free(gyatt_dir);
            
            // Send refs
            send(client_socket, "OK REFS\n", 8, 0);
            
            // TODO: Actually list the refs
            // For now, just send end marker
            send(client_socket, "END\n", 4, 0);
            
        } else if (strncmp(buffer, CMD_GET_OBJECT, strlen(CMD_GET_OBJECT)) == 0) {
            // Get object by hash
            char hash_str[HASH_HEX_SIZE + 1];
            if (sscanf(buffer, "GET-OBJECT %40s", hash_str) == 1) {
                gyatt_hash_t hash;
                hex_to_hash(hash_str, &hash);
                
                // Read object
                object_type_t type;
                size_t size;
                void *data = object_read(&hash, &type, &size);
                
                if (data) {
                    char response[256];
                    snprintf(response, sizeof(response), "OK OBJECT %zu\n", size);
                    send(client_socket, response, strlen(response), 0);
                    send(client_socket, data, size, 0);
                    free(data);
                } else {
                    const char *err = "ERROR Object not found\n";
                    send(client_socket, err, strlen(err), 0);
                }
            } else {
                const char *err = "ERROR Invalid hash\n";
                send(client_socket, err, strlen(err), 0);
            }
            
        } else if (strncmp(buffer, CMD_PUT_OBJECT, strlen(CMD_PUT_OBJECT)) == 0) {
            // Receive object
            size_t obj_size;
            object_type_t type;
            if (sscanf(buffer, "PUT-OBJECT %d %zu", (int *)&type, &obj_size) == 2) {
                // Allocate buffer for object data
                void *obj_data = malloc(obj_size);
                if (!obj_data) {
                    const char *err = "ERROR Out of memory\n";
                    send(client_socket, err, strlen(err), 0);
                    continue;
                }
                
                // Receive object data
                size_t received = 0;
                while (received < obj_size) {
                    ssize_t n = recv(client_socket, (char *)obj_data + received, 
                                    obj_size - received, 0);
                    if (n <= 0) break;
                    received += n;
                }
                
                if (received == obj_size) {
                    // Write object
                    gyatt_hash_t hash;
                    if (object_write(obj_data, obj_size, type, &hash) == 0) {
                        char hash_str[HASH_HEX_SIZE + 1];
                        hash_to_hex(&hash, hash_str);
                        
                        char response[256];
                        snprintf(response, sizeof(response), "OK STORED %s\n", hash_str);
                        send(client_socket, response, strlen(response), 0);
                    } else {
                        const char *err = "ERROR Failed to write object\n";
                        send(client_socket, err, strlen(err), 0);
                    }
                } else {
                    const char *err = "ERROR Incomplete data\n";
                    send(client_socket, err, strlen(err), 0);
                }
                
                free(obj_data);
            } else {
                const char *err = "ERROR Invalid PUT-OBJECT command\n";
                send(client_socket, err, strlen(err), 0);
            }
            
        } else if (strncmp(buffer, CMD_QUIT, strlen(CMD_QUIT)) == 0) {
            const char *response = "BYE\n";
            send(client_socket, response, strlen(response), 0);
            break;
            
        } else {
            const char *err = "ERROR Unknown command\n";
            send(client_socket, err, strlen(err), 0);
        }
    }
    
    close(client_socket);
}

int cmd_server(int argc, char *argv[]) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Error: Not a Gyatt repository\n");
        fprintf(stderr, "Run 'gyatt init' first to create a repository\n");
        return 1;
    }
    
    // Parse port
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Error: Invalid port number\n");
            return 1;
        }
    }
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(server_socket);
        return 1;
    }
    
    // Bind to port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        return 1;
    }
    
    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        return 1;
    }
    
    char *repo_path = get_gyatt_dir();
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║           GYATT SERVER - STARTED                       ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Repository: %s\n", repo_path ? repo_path : ".");
    printf("Listening on: 0.0.0.0:%d\n", port);
    printf("Server is ready to accept connections!\n");
    printf("\n");
    printf("Commands:\n");
    printf("  HELLO          - Handshake\n");
    printf("  LIST-REFS      - List all branches\n");
    printf("  GET-OBJECT     - Fetch an object\n");
    printf("  PUT-OBJECT     - Store an object\n");
    printf("  QUIT           - Close connection\n");
    printf("\n");
    printf("Press Ctrl+C to stop the server\n");
    printf("════════════════════════════════════════════════════════\n\n");
    free(repo_path);
    
    // Accept connections
    while (server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Set timeout on accept
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                continue;  // Timeout, check if server should still run
            }
            if (server_running) {
                perror("Error accepting connection");
            }
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("✓ Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        
        // Handle client (blocking for now, could fork/thread for multiple clients)
        handle_client(client_socket);
        
        printf("✗ Client disconnected\n");
    }
    
    close(server_socket);
    printf("\n✓ Server stopped\n");
    
    return 0;
}
