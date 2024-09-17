#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include <pthread.h>

#define PORT 8080
#define BUFFER_LEN 30000

// Function to handle a single client connection
void *handle_client(void *socket_desc) {

    int new_socket = *(int*)socket_desc;
    char buffer[BUFFER_LEN] = {0};

    // Read client request
    read(new_socket, buffer, BUFFER_LEN);
    printf("%s\n", buffer);

    // Send HTTP response
    char *response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 13\n\nHello, World!\n";
    write(new_socket, response, strlen(response));

    // Close connection
    close(new_socket);

    // Free the socket descriptor
    free(socket_desc);
    pthread_exit(NULL);    
}

int main() {

    // File descriptors, socket struct
    int server_fd, *new_socket;
    struct sockaddr_in address;

    int addrlen = sizeof(address);
    char buffer[30000] = {0};

    // 1. Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed!");
        exit(EXIT_FAILURE);
    }

    // 2. Bind to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed!");
        exit(EXIT_FAILURE);
    }

    // 3. Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed!");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // 4. Accept and handle incoming connections in new threads
    while(1) {

        new_socket = malloc(sizeof(int));

        if((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed!");
            free(new_socket); 
            exit(EXIT_FAILURE);
        }

        // 5. Create a new thread for each client
        pthread_t client_thread;
        
        if(pthread_create(&client_thread, NULL, handle_client, (void*)new_socket) < 0) {
            perror("Thread creation failed!");
            free(new_socket);
            exit(EXIT_FAILURE);
        }

        // Deetach the thread for clean up
        pthread_detach(client_thread);
    }

    return 0; 
}