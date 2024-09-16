#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

int main() {

    // File descriptors, socket struct
    int server_fd, new_socket;
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

    // 4. Accept and handle incoming connection
    while(1) {
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // 5. Read client request
        read(new_socket, buffer, 30000);
        printf("%s\n",buffer);

        // 6. Send HTTP response
        char *response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 13\n\nHello, World!\n";
        write(new_socket, response, strlen(response));

        // 7. Close connection
        close(new_socket);
    }

    return 0; 
}