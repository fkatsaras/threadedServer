#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <pthread.h>

#define PORT 8080
#define BUFFER_LEN 30000
#define WEB_ROOT "www"

// Function to send HTTP response
void send_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char response[4096]; // Might need fixing
    int content_length = strlen(body);

    snprintf(response,
                sizeof(response),
                "HTTP/1.1 %s\nContent-type: %s\nContent-Length: %d\n\n%s",
                status, content_type, content_length, body);

    write(client_socket, response, strlen(response));
}

// Function to parse an HTTP response
int parse_request(const char *buffer, char *method, char *path, char *version) {
    // Extract the request line
    return sscanf(buffer, "%s %s %s", method, path, version);
}

// Function to read a file and return its contents
char* read_file(const char *file_path, int *file_size) {
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        return NULL;
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        close(file_fd);
        return NULL;
    }

    *file_size = file_stat.st_size;
    char *file_buffer = malloc(*file_size);
    if (file_buffer == NULL) {
        close(file_fd);
        return NULL;
    }

    ssize_t bytes_read = read(file_fd, file_buffer, *file_size);
    close(file_fd);
    if (bytes_read != *file_size) {
        free(file_buffer);
        return NULL;
    }

    return file_buffer;
}


// Function to handle a single client connection
void *handle_client(void *socket_desc) {

    int new_socket = *(int*)socket_desc;
    char buffer[BUFFER_LEN] = {0};

    char method[16], path[1024], version[16]; // Array lengths might need adjustment/ better logic
    char file_path[2048];

    // Read client request
    ssize_t bytes_read = read(new_socket, buffer, BUFFER_LEN);
    if (bytes_read <= 0) {
        perror("Read failed or client disconnected");
        close(new_socket);
        free(socket_desc);
        pthread_exit(NULL);
    }

    // Print the received request
    printf("Received request:\n%s\n", buffer);

    // Parse the request
    if (parse_request(buffer, method, path, version) != 3) {
        send_response(new_socket, "400 Bad request", "text/plain", "Bad request");
        close(new_socket);
        free(socket_desc);
        pthread_exit(NULL);
    }

    // Handle GET requests
    if (strcmp(method, "GET") != 0) {
        send_response(new_socket, "405 Method not allowed", "text/plain", "Method not allowed");
        close(new_socket);
        free(socket_desc);
        exit(EXIT_FAILURE);
    }

    // Sanitize the path
    if (path[0] == '/') {
        memmove(path, path + 1, strlen(path)); // Remove leading '/'
    }

    snprintf(file_path, sizeof(file_path), "%s/%s", WEB_ROOT, path);
    if (strcmp(path, "") == 0) {
        snprintf(file_path, sizeof(file_path), "%s/index.html", WEB_ROOT);
    }

    // Read the file
    int file_size;
    char *file_buffer = read_file(file_path, &file_size);
    if (file_buffer) {

        send_response(new_socket, "200 OK", "text/html", file_buffer);
        free(file_buffer);
    }
    else {
        send_response(new_socket, "404 Not Found", "text/plain", "File Not Found");
    }

    // Close connection
    close(new_socket);
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