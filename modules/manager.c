#include "../headers/user.h"
#include "../headers/accounts.h"
#include "../headers/manager.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int activate_account(int cust_id){
    int fd;
    Account temp_account;
    struct flock lock;

    fd = open(ACCOUNT_FILE, O_RDWR);
    if (fd == -1) {
        perror("Error opening loan file");
        return -1;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    lseek(fd, 0, SEEK_SET);
    while (read(fd, &temp_account, sizeof(Account)) > 0) {
        if (temp_account.user_ID == cust_id) {
            lseek(fd, -sizeof(Account), SEEK_CUR);
            temp_account.active = !temp_account.active;
            if (write(fd, &temp_account, sizeof(Account)) == -1) {
                perror("Error updating account");
                close(fd);
                return -1;
            }

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            return 0; // Success
        }
    }

    printf("account ID %d not found for update.\n", cust_id);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return -1; // account not found
}

int assign_loan_to_employee(int loan_id, int emp_id){
    Loan* loan = read_loan(loan_id);
    if(loan == NULL){
        printf("No such loan found!\n");
        return -1;
    }
    if(loan->status != 0){
        printf("This loan is not pending!\n");
        return 1;
    }
    loan->employee_ID = emp_id;
    if(update_loan(*loan) != 0) return -1;
    return 1;
    
}


char* read_all_feedback() {
    int fd = open(FEEDBACK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening feedback file");
        return NULL;
    }

    // Allocate enough memory to hold the entire file
    char *feedback_content = malloc(1024);
    if (feedback_content == NULL) {
        perror("Error allocating memory");
        close(fd);
        return NULL;
    }

    // Read the file content into the allocated memory
    lseek(fd, 0, SEEK_SET);  // Go back to the beginning of the file
    ssize_t bytes_read = read(fd, feedback_content, 1023);
    if (bytes_read == -1) {
        perror("Error reading feedback file");
        free(feedback_content);
        close(fd);
        return NULL;
    }

    feedback_content[bytes_read] = '\0';  // Null-terminate the string

    close(fd);
    return feedback_content;
}

void manager_menu(int sockfd, User *manager) {
    int choice;
    char buffer[1024];
    int customer_id, loan_id, employee_id;
    
    while (1) {
        printf("\nManager Menu:\n");
        printf("1. Activate/Deactivate Customer accounts\n");
        printf("2. Show all pending Loans\n");
        printf("3. Assign Loan Applications to employees\n");
        printf("4. Review Feedback\n");
        printf("5. Change Password\n");
        printf("6. Logout\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter Customer ID: ");
                scanf("%d", &customer_id);
                snprintf(buffer, sizeof(buffer), "TOGGLE_ACCOUNT %d", customer_id);
                write(sockfd, buffer, strlen(buffer));  // Send request to the server

                // Receive response
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Show the result from the server
                break;

            case 2:
                snprintf(buffer, sizeof(buffer), "SHOW_PENDING_LOANS");
                write(sockfd, buffer, strlen(buffer));  // Send request to the server

                // Receive response (pending loans list)
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Show the result from the server
                break;

            case 3:
                printf("Enter Loan ID to assign: ");
                scanf("%d", &loan_id);
                printf("Enter Employee ID to assign: ");
                scanf("%d", &employee_id);
                snprintf(buffer, sizeof(buffer), "ASSIGN_LOAN %d %d", loan_id, employee_id);
                write(sockfd, buffer, strlen(buffer));  // Send request to the server
                break;

            case 4:
                snprintf(buffer, sizeof(buffer), "REVIEW_FEEDBACK");
                write(sockfd, buffer, strlen(buffer));  // Send request to the server
                break;

            case 5:{
                printf("Enter your new password: ");
                getchar();  // To clear out the newline from previous input
                char newpassword[MAX_PASSWORD_LEN];
                scanf("%s", newpassword);
                snprintf(buffer, sizeof(buffer), "CHANGE_PASSWORD %s", newpassword);
                write(sockfd, buffer, strlen(buffer));
                break;}

            case 6:
                snprintf(buffer, sizeof(buffer), "LOGOUT");
                write(sockfd, buffer, strlen(buffer));  // Send request to the server
                return;

            case 7:
                snprintf(buffer, sizeof(buffer), "EXIT");
                write(sockfd, buffer, strlen(buffer));  // Send request to the server
                exit(EXIT_SUCCESS);

            default:
                printf("Invalid choice, please try again.\n");
        }
    }
}

int handle_manager_request(int client_sock, User *manager) {
    char buffer[1024];
    int customer_id, loan_id, employee_id;
    ssize_t bytes_received;
    int bytes_received = read(client_sock, buffer, sizeof(buffer) - 1);
    buffer[bytes_received] = '\0';


    // Parse the request and handle accordingly
    if (strncmp(buffer, "TOGGLE_ACCOUNT", 14) == 0) {
        sscanf(buffer, "TOGGLE_ACCOUNT %d", &customer_id);
        if (activate_account(customer_id) == 0) {
            snprintf(buffer, sizeof(buffer), "Customer account updated successfully.");
        } else {
            snprintf(buffer, sizeof(buffer), "Failed to update customer account.");
        }
        write(client_sock, buffer, strlen(buffer));

    } else if (strncmp(buffer, "SHOW_PENDING_LOANS", 18) == 0) {
        char loans[1024];
        get_pending_loans(loans, sizeof(loans));  // Populate loans with pending loan details
        write(client_sock, loans, strlen(loans));

    } else if (strncmp(buffer, "ASSIGN_LOAN", 11) == 0) {
        sscanf(buffer, "ASSIGN_LOAN %d %d", &loan_id, &employee_id);
        if (assign_loan_to_employee(loan_id, employee_id) == 0) {
            snprintf(buffer, sizeof(buffer), "Loan assigned successfully.");
        } else {
            snprintf(buffer, sizeof(buffer), "Failed to assign loan.");
        }
        write(client_sock, buffer, strlen(buffer));

    } else if (strncmp(buffer, "REVIEW_FEEDBACK", 15) == 0) {
        char *feedback;
        feedback = read_all_feedback();  // Populate feedback with feedback details
        write(client_sock, feedback, strlen(feedback));

    } else if (strncmp(buffer, "LOGOUT", 6) == 0) {
        snprintf(buffer, sizeof(buffer), "Logging out...");
        logout(manager);
        return 0;
    }
    else if (strncmp(buffer, "EXIT", 4) == 0) {
        snprintf(buffer, sizeof(buffer), "Exiting...");
        logout(manager);
        exit(EXIT_SUCCESS);
    }
    else {
        snprintf(buffer, sizeof(buffer), "Invalid request!");
    }
    // Send the response back to the client
    return 1;
}



/*
void manager_menu(int fd, User *manager){
    int choice;
    while(1){
        printf("\nAdmin Menu:\n");
        printf("1. Activate/Deactivate Customer accounts\n");
        printf("2. Show all pending Loans\n");
        printf("3. Assign Loan Applications to employees\n");
        printf("4. Review Feedback\n");
        printf("5. Change Password\n");
        printf("6. Logout\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:{
            int cust_id, active;
            printf("Enter customer ID: ");
            scanf("%d", &cust_id);
            printf("Activate(1) or Deactivate(0): ");
            scanf("%d", &active);
            activate_account(cust_id, active);
            break;}
        case 2:{
            display_pending_loans();
            break;}
        case 3:{
            int loan_id, emp_id;
            printf("Enter loan ID: ");
            scanf("%d", &loan_id);
            printf("Enter employee to assign it to: ");
            scanf("%d", &emp_id );
            assign_loan_to_employee(loan_id, emp_id);
            break;}
        case 4:{
            display_all_feedback();
            break;
        }
        case 5:{
            change_password(fd, manager);
            break;
        }
        case 6:{
            logout(manager, fd);
            return;
        }
        case 7:{
            logout(manager, fd);
            exit(EXIT_SUCCESS);
        }
        default:
            break;
        }
    }
}
*/

