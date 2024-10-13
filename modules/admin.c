#include "../headers/user.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void add_new_employee(int fd) {
    User new_employee;
    printf("Enter new employee's username: ");
    scanf("%s", new_employee.username);
    printf("Enter password: ");
    scanf("%s", new_employee.password);
    printf("Enter ID: ");
    scanf("%d", &new_employee.ID);
    new_employee.role = 2; // Employee role

    lseek(fd, 0, SEEK_END); // Move to the end of file to add a new record
    write(fd, &new_employee, sizeof(User));
    printf("New employee added successfully.\n");
}

void modify_user(int fd, const char *username) {
    User temp_user;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &temp_user, sizeof(User)) > 0) {
        if (strcmp(temp_user.username, username) == 0) {
            printf("Enter new password for %s: ", username);
            scanf("%s", temp_user.password);
            printf("Enter ID: ");
            scanf("%d", &temp_user.ID);
            lseek(fd, -sizeof(User), SEEK_CUR);
            write(fd, &temp_user, sizeof(User));
            printf("User details updated.\n");
            return;
        }
    }
    printf("User not found.\n");
}

void modify_role(int fd, const char *username, int new_role){
    User temp_user;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &temp_user, sizeof(User)) > 0) {
        if (strcmp(temp_user.username, username) == 0) {
            temp_user.role = new_role;
            lseek(fd, -sizeof(User), SEEK_CUR);
            write(fd, &temp_user, sizeof(User));
            printf("User details updated.\n");
            return;
        }
    }
    printf("User not found.\n");
}

void change_password(int fd, User *admin_user){
    modify_user(fd, admin_user->username);
}

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
            char username[MAX_USERNAME_LEN];
            printf("Enter the username to modify: ");
            scanf("%s", username);
            modify_user(fd, username);
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