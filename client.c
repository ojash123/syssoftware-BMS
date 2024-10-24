#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "headers/bank.h"
#define PORT 8080



int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char username[50], password[50], buffer[1024];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(1);
    }

    // Define the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Login process
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    // Send login details to the server
    snprintf(buffer, sizeof(buffer), "LOGIN %s %s", username, password);
    send(sockfd, buffer, strlen(buffer), 0);

    // Read server response
    read(sockfd, buffer, sizeof(buffer));

    // Check if login was successful
    if (strstr(buffer, "Login successful") != NULL) {
        char name[50];
        int role, id;
        sscanf(buffer, "Login successful! Welcome %s %d %d.\n", name, &role, &id);
        printf("Login successful! Welcome %s, userID: %d", name, id);
        switch (role)
        {
        case 1:
            customer_menu(sockfd);
            break;
        case 2: 
            employee_menu(sockfd);
            break;
        case 3:
            printf("manager\n");
            manager_menu(sockfd);
            break;
        case 4:
            admin_menu(sockfd);
            break;
        default:
            break;
        }
    } else {
        printf("Login failed. Exiting...\n");
    }

    // Close the socket after logout
    close(sockfd);
    return 0;
}