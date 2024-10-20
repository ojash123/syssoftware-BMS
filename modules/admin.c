#include "../headers/user.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int add_new_employee(User new_employee) {
    new_employee.ID = generate_unique_user_id();
    new_employee.role = 2; // Employee role
    initialize_semaphore(&new_employee);
    if(save_user_to_file(new_employee)!= 0)return -1;
    
    printf("New employee added successfully.\n");
    return 0;
}



int modify_role(int user_id, int new_role){
    User *temp_user;
    temp_user = read_user(user_id);
    temp_user->role = new_role;
    if (update_user(*temp_user) != 0)
        return -1;
    return 0;
}

void admin_menu(int sockfd) {
    char buffer[256];
    int choice;
    int id, role;

    while (1) {
        printf("\nAdmin Menu:\n");
        printf("1. Add New Bank Employee\n");
        printf("2. Modify Customer/Employee Details\n");
        printf("3. Manage User Roles\n");
        printf("4. Change Password\n");
        printf("5. Logout\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:{
                char name[50], password[50];
                
                printf("Enter new employee's name: ");
                scanf("%s", name); 
                printf("Enter new employee's password: ");
                scanf("%s", password);  
                snprintf(buffer, sizeof(buffer), "ADD_EMPLOYEE %s %s", name, password);
                write(sockfd, buffer, strlen(buffer));
                break;
            }

            case 2:{
                int id;
                char new_name[50], new_password[50];            
                printf("Enter user ID to modify: ");
                scanf("%d", &id);  
                printf("Enter new user's name: ");
                scanf("%s", new_name);  
                printf("Enter new user's password: ");
                scanf("%s", new_password);  
                snprintf(buffer, sizeof(buffer), "MODIFY_USER %d %s %s", id, new_name, new_password);
                write(sockfd, buffer, strlen(buffer));
                break;
            }

            case 3:
                // Request to manage user roles
                printf("Enter the ID to manage role: ");
                scanf("%d", &id);
                printf("Enter the new role: ");
                scanf("%d", &role);
                snprintf(buffer, sizeof(buffer), "MANAGE_ROLES %d %d", id, role);
                write(sockfd, buffer, strlen(buffer));
                break;

            case 4:
                // Request to change password
                {
                printf("Enter your new password: ");
                getchar();  // To clear out the newline from previous input
                char newpassword[MAX_PASSWORD_LEN];
                scanf("%s", newpassword);
                snprintf(buffer, sizeof(buffer), "CHANGE_PASSWORD %s", newpassword);
                write(sockfd, buffer, strlen(buffer));
                break;}


            case 5:
                // Logout request
                snprintf(buffer, sizeof(buffer), "LOGOUT");
                write(sockfd, buffer, strlen(buffer));
                return;

            case 6:
                // Exit request
                snprintf(buffer, sizeof(buffer), "EXIT");
                write(sockfd, buffer, strlen(buffer));
                exit(0);

            default:
                printf("Invalid choice, please try again.\n");
        }

        // Receive the server response
        int bytes_received = read(sockfd, buffer, sizeof(buffer) - 1);
        buffer[bytes_received] = '\0';
        printf("Server Response: %s\n", buffer);
    }
}

int handle_admin_request(int client_sock, User *admin) {
    char buffer[1024];
    char response[1024];
    int id, role;

    // Read the request from the client
    int bytes_received = read(client_sock, buffer, sizeof(buffer) - 1);
    buffer[bytes_received] = '\0';

    if (strncmp(buffer, "ADD_EMPLOYEE", 12) == 0) {
        // Handle adding a new employee
        char name[50], password[50];
        sscanf(buffer, "ADD_CUSTOMER %s %s", name, password);
        User new_customer;
        strcpy(new_customer.username, name);
        strcpy(new_customer.password, password);
        if (add_new_employee(new_customer) == 0) {
            snprintf(response, sizeof(response), "New employee added successfully.");
        } else {
            snprintf(response, sizeof(response), "Failed to add new employee.");
        }
    } else if (strncmp(buffer, "MODIFY_USER", 15) == 0) {
        char name[50], password[50];
        sscanf(buffer, "MODIFY_USER %d %s %s", name, password);
        User *user = read_user(id);
        strcpy(user->username, name);
        strcpy(user->password, password);
        
        if (update_user(*user) == 0) {
            snprintf(buffer, sizeof(buffer), "Customer modified successfully!");
        } else {
            snprintf(buffer, sizeof(buffer), "Failed to modify customer!");
        }
    } else if (sscanf(buffer, "MANAGE_ROLES %d, %d", &id, &role) == 1) {
        // Handle managing user roles
        if (modify_role(id, role) == 0) {
            snprintf(response, sizeof(response), "User %d role updated successfully.", id);
        } else {
            snprintf(response, sizeof(response), "Failed to update user %d role.", id);
        }
    } else if (strncmp(buffer, "CHANGE_PASSWORD", 15) == 0) {
        char *newpassword;
        newpassword = buffer + sizeof("CHANGE_PASSWORD");
        strcpy(admin->password, newpassword);
        if(update_user(*admin) == 0){
            snprintf(buffer, sizeof(buffer), "Password changed!");
        }else{
            snprintf(buffer, sizeof(buffer), "Password change failed");
        }
    } else if (strncmp(buffer, "LOGOUT", 6) == 0) {
        snprintf(buffer, sizeof(buffer), "Logging out...");
        logout(admin);
        return 0;
    }
    else if (strncmp(buffer, "EXIT", 4) == 0) {
        snprintf(buffer, sizeof(buffer), "Exiting...");
        logout(admin);
        exit(EXIT_SUCCESS);
    }
    else {
        snprintf(buffer, sizeof(buffer), "Invalid request!");
    }
    // Send the response back to the client
    write(client_sock, response, strlen(response));
    return 1;
}


/*
void admin_menu(int fd, User *admin_user){
    int choice;
    while(1){
        printf("\nAdmin Menu:\n");
        printf("1. Add New Bank Employee\n");
        printf("2. Modify Customer/Employee Details\n");
        printf("3. Manage User Roles\n");
        printf("4. Change Password\n");
        printf("5. Logout\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:{
            add_new_employee(fd);
            break;}
        case 2:{
            int id;
            printf("Enter the id to modify: ");
            scanf("%d", &id);
            modify_user(fd, id);
            break;}
        case 3:{
            char username[MAX_USERNAME_LEN];
            int new_role;
            printf("Enter the username to modify: ");
            scanf("%s", username);
            printf("Enter new role: ");
            scanf("%d", &new_role);
            modify_role(fd, username, new_role);
            break;}
        case 4:{
            change_password(fd, admin_user);
            break;
        }
        case 5:{
            logout(admin_user, fd);
            return;
        }
        case 6:{
            logout(admin_user, fd);
            exit(EXIT_SUCCESS);
        }
        default:
            break;
        }
    }
}
*/
