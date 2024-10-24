#include "../headers/user.h"
#include "../headers/accounts.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int add_new_customer(User new_customer) {
    new_customer.ID = generate_unique_user_id();
    new_customer.role = 1; // Customer role
    initialize_semaphore(&new_customer);
    Account useracc;
    useracc.user_ID = new_customer.ID;
    useracc.active = true;
    useracc.balance = 0;
    if(save_acc_to_file(useracc) != 0) return -1;
    if(save_user_to_file(new_customer) != 0) return -1;
    
    printf("New customer added successfully.\n");
    return 0;
}

void view_assigned_loans(int employee_id){
    int fd;
    Loan loan;

    fd = open(LOAN_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening loan file");
        return;
    }

    printf("Pending Loans:\n");
    lseek(fd, 0, SEEK_SET);  
    while (read(fd, &loan, sizeof(Loan)) > 0) {
        if (loan.status == 0 && loan.employee_ID == employee_id) {
            printf("Loan ID: %d, Customer ID: %d, Amount: %.2f\n",
                   loan.loan_ID, loan.customer_ID, loan.loan_amount);
            
        }
    }

    close(fd);
}
void send_assigned_loans(char *buffer, size_t buffer_size, int employee_id) {
    int fd;
    Loan loan;
    ssize_t bytes_read;
    size_t offset = 0;

    fd = open(LOAN_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening loan file");
        return;
    }

    // Ensure buffer starts with an empty string
    buffer[0] = '\0';

    offset += snprintf(buffer + offset, buffer_size - offset, "Assigned Loans:\n");

    lseek(fd, 0, SEEK_SET);
    while ((bytes_read = read(fd, &loan, sizeof(Loan))) > 0) {
        if (loan.status == 0 && loan.employee_ID == employee_id) {
            // Check if there is enough space in the buffer before adding more
            if (offset < buffer_size) {
                offset += snprintf(buffer + offset, buffer_size - offset,
                                   "Loan ID: %d, Customer ID: %d, Amount: %.2f\n",
                                   loan.loan_ID, loan.customer_ID, loan.loan_amount);

                if (loan.employee_ID == -1) {
                    offset += snprintf(buffer + offset, buffer_size - offset, "Status: Unassigned\n");
                } else {
                    offset += snprintf(buffer + offset, buffer_size - offset, "Assigned Employee ID: %d\n", loan.employee_ID);
                }
            } else {
                // If buffer size is exceeded, truncate and stop further writing
                snprintf(buffer + buffer_size - 5, 5, "...\n");
                break;
            }
        }
    }

    if (bytes_read == -1) {
        perror("Error reading loan file");
    }
    printf("%s\n", buffer);
    close(fd);
}

void receive_assigned_loans(int sockfd) {
    char buffer[256];
    ssize_t bytes_received;

    // Send request to server
    snprintf(buffer, sizeof(buffer), "VIEW_ASSIGNED_LOANS\n");
    write(sockfd, buffer, strlen(buffer));

    // Read the loan information from server until "END_OF_LOANS" is received
    while ((bytes_received = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the received data

        // Check for end of loan data
        if (strstr(buffer, "END_OF_LOANS") != NULL) {
            break;  // End the loop when no more loans are sent
        }

        // Print the loan details
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        perror("Error reading from server");
    }
}


int process_loan(int loan_id, int approve){
    Loan* loan = read_loan(loan_id);
    if(loan == NULL){
        printf("No such loan found!\n");
        return -1;
    }
    if(loan->status != 0){
        printf("This loan is not pending!\n");
        return 1;
    }
    if(approve){
        loan->status = 1;
        if(update_loan(*loan)!= 0) return -1;
        if(update_balance(loan->customer_ID, loan->loan_amount, 5)!= 0) return -1;
    }else{
        loan->status = -1;
        if(update_loan(*loan)!= 0) return -1;
    }
    return 1;
    
}
/*
void employee_menu_test(int fd, User *employee){
    int choice;
    while(1){
        printf("\nEmployee Menu:\n");
        printf("1. Add New Customer\n");
        printf("2. Modify Customer Details\n");
        printf("3. View Customer transactions\n");
        printf("4. View Assigned Loans\n");
        printf("5. Approve/deny Loan\n");
        printf("6. Change Password\n");
        printf("7. Logout\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:{
            add_new_customer(fd);
            break;}
        case 2:{
            int id;
            printf("Enter the id to modify: ");
            scanf("%d", &id);
            modify_user(fd, id);
            break;}
        case 3: {
            int customer_id;
            printf("Enter Customer ID: ");
            scanf("%d", &customer_id);
            view_transactions(customer_id);
            break;
        }
        case 4:{
            view_assigned_loans(employee->ID);
            break;}
        case 5:{
            int loanid, approve;
            printf("Enter Loan ID: ");
            scanf("%d", &loanid);
            printf("Approve(1) or deny(0): ");
            scanf("%d" , &approve);
            process_loan(loanid, approve);
            break;

        }
        case 6:{
            change_password(fd, employee);
            break;
        }
        case 7:{
            logout(employee, fd);
            return;
        }
        case 8:{
            logout(employee, fd);
            exit(EXIT_SUCCESS);
        }
        default:
            break;
        }
    }
}
*/
void employee_menu(int sockfd) {
    char buffer[256];
    int choice, id, loanid, approve;
    
    while (1) {
        printf("\nEmployee Menu:\n");
        printf("1. Add New Customer\n");
        printf("2. Modify Customer Details\n");
        printf("3. View Customer Transactions\n");
        printf("4. View Assigned Loans\n");
        printf("5. Approve/Deny Loan\n");
        printf("6. Change Password\n");
        printf("7. Logout\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:{
                char name[50], password[50];
                
                printf("Enter new customer's name: ");
                scanf("%s", name); 
                printf("Enter new customer's password: ");
                scanf("%s", password);  
                snprintf(buffer, sizeof(buffer), "ADD_CUSTOMER %s %s", name, password);
                write(sockfd, buffer, strlen(buffer));
            }
            break;
            
            case 2:{
                int id;
                char new_name[50], new_password[50];            
                printf("Enter customer ID to modify: ");
                scanf("%d", &id);  
                printf("Enter new customer's name: ");
                scanf("%s", new_name);  
                printf("Enter new customer's password: ");
                scanf("%s", new_password);  
                snprintf(buffer, sizeof(buffer), "MODIFY_CUSTOMER %d %s %s", id, new_name, new_password);
                write(sockfd, buffer, strlen(buffer));
                break;
            }
            case 3:
                printf("Enter Customer ID to view transactions: ");
                scanf("%d", &id);
                snprintf(buffer, sizeof(buffer), "VIEW_TRANSACTIONS %d", id);
                write(sockfd, buffer, strlen(buffer));
                break;

            case 4:
                snprintf(buffer, sizeof(buffer), "VIEW_ASSIGNED_LOANS\n");
                write(sockfd, buffer, strlen(buffer));
                break;

            case 5:
                printf("Enter Loan ID: ");
                scanf("%d", &loanid);
                printf("Approve(1) or deny(0): ");
                scanf("%d", &approve);
                snprintf(buffer, sizeof(buffer), "PROCESS_LOAN %d %d", loanid, approve);
                write(sockfd, buffer, strlen(buffer));
                break;

            case 6:{
                printf("Enter your new password: ");
                getchar();  // To clear out the newline from previous input
                char newpassword[MAX_PASSWORD_LEN];
                scanf("%s", newpassword);
                snprintf(buffer, sizeof(buffer), "CHANGE_PASSWORD %s", newpassword);
                write(sockfd, buffer, strlen(buffer));
                break;}

            case 7:
                snprintf(buffer, sizeof(buffer), "LOGOUT");
                write(sockfd, buffer, strlen(buffer));
                return;

            case 8:
                snprintf(buffer, sizeof(buffer), "EXIT");
                write(sockfd, buffer, strlen(buffer));
                exit(0);
                
            default:
                printf("Invalid choice\n");
        }

        // Read server response
        int n  = read(sockfd, buffer, sizeof(buffer) - 1);
        
        buffer[n] = '\0';
        printf("Server: %s\n", buffer);
    }
}

int handle_employee_request(int client_sock, User *employee) {
    char buffer[256];
    int n, id, loanid, approve;


    n = read(client_sock, buffer, sizeof(buffer) - 1);
    buffer[n] = '\0';

    if (strncmp(buffer, "ADD_CUSTOMER", 12) == 0) {
        //read newcustomer details
        char name[50], password[50];
        sscanf(buffer, "ADD_CUSTOMER %s %s", name, password);
        User new_customer;
        strcpy(new_customer.username, name);
        strcpy(new_customer.password, password);
        if (add_new_customer(new_customer) == 0) {
            snprintf(buffer, sizeof(buffer), "Customer added successfully!");
        } else {
            snprintf(buffer, sizeof(buffer), "Failed to add customer!");
        }
    }
    else if (strncmp(buffer, "MODIFY_CUSTOMER", 15) == 0) {
        char name[50], password[50];
        sscanf(buffer, "MODIFY_CUSTOMER %d %s %s", &id, name, password);
        User *user = read_user(id);
        strcpy(user->username, name);
        strcpy(user->password, password);
        
        if (update_user(*user) == 0) {
            snprintf(buffer, sizeof(buffer), "Customer modified successfully!");
        } else {
            snprintf(buffer, sizeof(buffer), "Failed to modify customer!");
        }
    }
    else if (sscanf(buffer, "VIEW_TRANSACTIONS %d", &id) == 1) {
        view_transaction_history(buffer, id);
    }
    else if (sscanf(buffer, "VIEW_ASSIGNED_LOANS %d", &employee->ID) == 1) {
        strcpy(buffer, "");
        send_assigned_loans(buffer,sizeof(buffer), employee->ID);
        snprintf(buffer, sizeof(buffer), "Assigned loans");
    }
    else if (sscanf(buffer, "PROCESS_LOAN %d %d", &loanid, &approve) == 2) {
        if (process_loan(loanid, approve) == 0) {
            snprintf(buffer, sizeof(buffer), "Loan processed successfully!");
        } else {
            snprintf(buffer, sizeof(buffer), "Failed to process loan!");
        }
    }
    else if (strncmp(buffer, "CHANGE_PASSWORD", 15) == 0) {
        char *newpassword;
        newpassword = buffer + sizeof("CHANGE_PASSWORD");
        strcpy(employee->password, newpassword);
        if(update_user(*employee) == 0){
            snprintf(buffer, sizeof(buffer), "Password changed!");
        }else{
            snprintf(buffer, sizeof(buffer), "Password change failed");
        }
    }
    else if (strncmp(buffer, "LOGOUT", 6) == 0) {
        snprintf(buffer, sizeof(buffer), "Logging out...");
        logout(employee);
        return 0;
    }
    else if (strncmp(buffer, "EXIT", 4) == 0) {
        snprintf(buffer, sizeof(buffer), "Exiting...");
        logout(employee);
        exit(EXIT_SUCCESS);
    }
    else {
        snprintf(buffer, sizeof(buffer), "Invalid request!");
    }

    // Send response back to client
    write(client_sock, buffer, strlen(buffer));
    return 1;
}

