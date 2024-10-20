#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "headers/bank.h"

#define PORT 8080

void handle_client(int client_fd);
void dummy_users(){
    User user1 = {.ID = 0, .username = "employee", .password = "password", .role = 2};
    //User user2 = {.ID = 2, .username = "user2", .password = "password", .role = 1};
    initialize_semaphore(&user1);
    //initialize_semaphore(&user2);
    int fd = open(USER_FILE, O_CREAT | O_RDWR | O_TRUNC, 0744);
    if (fd == -1) {
        perror("Failed to create file");
        return;
    }
    write(fd, &user1, sizeof(User));
    //write(fd, &user2, sizeof(User));
    close(fd);

}
int main() {
    dummy_users();
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections...\n");

    // Accept and handle incoming connections
    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        
        handle_client(client_fd);  // Handle the client connection
        close(client_fd);  // Close connection after client interaction is done
    }

    return 0;
}

// Function to handle client request (login + role-based menu)
void handle_client(int client_fd) {
    char buffer[1024] = {0};
    User logged_in_user;

    // Receive login information
    read(client_fd, buffer, sizeof(buffer));
    
    // Extract username and password
    char username[50], password[50];
    sscanf(buffer, "LOGIN %s %s", username, password);

    int user_fd = open(USER_FILE, O_RDWR);
    if (user_fd < 0) {
        perror("Error opening user file");
        exit(1);
    }

    // Call the login function (actual project function)
    if (login(&logged_in_user,  username, password) == 0) {
        // Send login success to the client
        snprintf(buffer, sizeof(buffer), "Login successful! Welcome %s %d %d.\n", logged_in_user.username, logged_in_user.role, logged_in_user.ID);
        write(client_fd, buffer, strlen(buffer));
        int keep_alive =1;
        // Keep processing customer requests until logout or exit
        if (logged_in_user.role == 1) {
            while (keep_alive) {
                // Handle the request from the customer
                keep_alive = handle_customer_request(client_fd, &logged_in_user, user_fd);
                // keep_alive will return 0 if the request is "logout" or "exit"
            }
        }else if (logged_in_user.role == 2)
        {
            while (keep_alive) {
                // Handle the request from the customer
                keep_alive = handle_employee_request(client_fd, &logged_in_user);
                // keep_alive will return 0 if the request is "logout" or "exit"
            }
        }else if (logged_in_user.role == 3)
        {
            while (keep_alive) {
                // Handle the request from the customer
                keep_alive = handle_manager_request(client_fd, &logged_in_user);
                // keep_alive will return 0 if the request is "logout" or "exit"
            }
        }else if (logged_in_user.role == 4)
        {
            while (keep_alive) {
                // Handle the request from the customer
                keep_alive = handle_admin_request(client_fd, &logged_in_user);
                // keep_alive will return 0 if the request is "logout" or "exit"
            }
        }
         
        // You can extend this to handle other roles like employee or admin if needed

    } else {
        // Send login failure to the client
        snprintf(buffer, sizeof(buffer), "Login failed! Invalid credentials.\n");
        write(client_fd, buffer, strlen(buffer));
    }

    close(user_fd);
}

