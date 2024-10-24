#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include "headers/bank.h"
#include <inttypes.h>

#define PORT 8080

void *handle_client(void *arg);
void dummy_users(){
    system("rm database/*");
    User user1 = {.ID = 0, .username = "admin", .password = "password", .role = 4};
    
    initialize_semaphore(&user1);
    //initialize_semaphore(&user2);
    int fd = open(USER_FILE, O_CREAT | O_RDWR | O_TRUNC, 0744);
    if (fd == -1) {
        perror("Failed to create file");
        return;
    }
    write(fd, &user1, sizeof(User));
    close(fd);
    User employee1 = { .username = "employee1", .password = "password" };
    User employee2 = { .username = "employee2", .password = "password" };
    add_new_employee(employee1);  // Function to add a new employee
    add_new_employee(employee2);

    printf("Two employees added: employee1 and employee2\n");

    // 2. Add three customers
    User customer1 = { .username = "customer1", .password = "password" };
    User customer2 = { .username = "customer2", .password = "password" };
    User customer3 = { .username = "customer3", .password = "password" };
    add_new_customer(customer1);  // Function to add a new customer
    add_new_customer(customer2);
    add_new_customer(customer3);

    printf("Three customers added: customer1, customer2, customer3\n");

    int employee2_id = 2 ;
    modify_role(employee2_id, 3);  
    printf("Employee2 promoted to manager\n");

    // 4. Request loans from customers
    int customer1_id = 3;
    int customer2_id = 4;
    int customer3_id = 5;
    apply_loan(customer1_id, 10000.00);  // Customer1 requests a loan of 10000
    apply_loan(customer2_id, 5000.00);   // Customer2 requests a loan of 5000
    printf("Loan requests made by customer1 and customer2\n");

    // 5. Simulate transactions (Deposits and Withdrawals)
    update_balance(customer1_id, 5000.00, 1);  // Deposit of 5000 by customer1
    update_balance(customer2_id, -2000.00, 2);  // Withdrawal of 2000 by customer2
    printf("Transactions made by customer1 and customer2\n");

    // 6. Add feedback from customers
    write_feedback(customer1_id, "Great banking service!");
    write_feedback(customer3_id, "Happy with the loan process.");
    printf("Feedback submitted by customer1 and customer3\n");




}

int main() {
    dummy_users();  // Initialize users
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

        // Create a new thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)(intptr_t)client_fd) != 0) {
            perror("Failed to create thread");
            close(client_fd);
        }

        // Detach the thread to allow it to clean up after itself
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}


// Function to handle client request (login + role-based menu)
void *handle_client(void *arg) {
    int client_fd = (intptr_t )arg;  // Retrieve the client file descriptor
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
    if (login(&logged_in_user, username, password) == 0) {
        // Send login success to the client
        snprintf(buffer, sizeof(buffer), "Login successful! Welcome %s %d %d.\n", logged_in_user.username, logged_in_user.role, logged_in_user.ID);
        write(client_fd, buffer, strlen(buffer));
        int keep_alive = 1;

        // Keep processing requests based on user role
        if (logged_in_user.role == 1) {
            while (keep_alive) {
                keep_alive = handle_customer_request(client_fd, &logged_in_user, user_fd);
            }
        } else if (logged_in_user.role == 2) {
            while (keep_alive) {
                keep_alive = handle_employee_request(client_fd, &logged_in_user);
            }
        } else if (logged_in_user.role == 3) {
            while (keep_alive) {
                keep_alive = handle_manager_request(client_fd, &logged_in_user);
            }
        } else if (logged_in_user.role == 4) {
            while (keep_alive) {
                keep_alive = handle_admin_request(client_fd, &logged_in_user);
            }
        }

    } else {
        // Send login failure to the client
        snprintf(buffer, sizeof(buffer), "Login failed! Invalid credentials.\n");
        write(client_fd, buffer, strlen(buffer));
    }

    close(user_fd);
}
