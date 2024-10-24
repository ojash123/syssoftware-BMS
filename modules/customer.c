#include "../headers/user.h"
#include "../headers/accounts.h"
#include "../headers/customer.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int write_feedback(int customer_id, const char* feedback) {
    int fd = open(FEEDBACK_FILE, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening feedback file");
        return -1;
    }

    // Create feedback string to write
    char feedback_entry[512];
    snprintf(feedback_entry, sizeof(feedback_entry), "Customer ID: %d\nFeedback: %s\n\n", customer_id, feedback);

    // Write the feedback to the file
    ssize_t bytes_written = write(fd, feedback_entry, strlen(feedback_entry));
    if (bytes_written == -1) {
        perror("Error writing feedback");
        close(fd);
        return -1;
    }

    printf("Feedback successfully saved!\n");
    close(fd);
    return 0;  
}

int transfer_funds(int from_customer_id, int to_customer_id, double amount) {
    if (amount <= 0) {
        printf("Invalid transfer amount.\n");
        return -1;
    }

    

    // Deduct from source customer
    if (update_balance(from_customer_id, amount * -1, 3) != 0) {
        printf("Insufficient balance or account inactive.\n");
        return -1;
    }

    // Add to destination customer
    if (update_balance(to_customer_id, amount, 4) != 0) {
        printf("Error crediting destination account.\n");
        return -1;
    }

    printf("Transfer of %.2f from customer %d to customer %d was successful.\n", amount, from_customer_id, to_customer_id);
    return 0; // Success
}

double get_balance(int id){
    int fd = open(ACCOUNT_FILE, O_RDONLY);
    if (fd == -1) {
        printf("Error opening accounts file");
        return -1;
    }

    Account account;
    struct flock lock;

    lock.l_type = F_RDLCK;  // Read lock
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(Account);
    lock.l_start = 0;  

    lseek(fd, 0, SEEK_SET);  
    while (read(fd, &account, sizeof(Account)) > 0) {
        if (account.user_ID == id) {
            lock.l_start = lseek(fd, -sizeof(Account), SEEK_CUR);  

            // Lock the current record
            if (fcntl(fd, F_SETLKW, &lock) == -1) {
                perror("Error locking account record");
                close(fd);
                return -1;
            }

            // Display the balance
            printf("Account ID: %d\n", account.user_ID);
            printf("Balance: %.2f\n", account.balance);
            double balance = account.balance;
            // Unlock the record
            lock.l_type = F_UNLCK;
            if (fcntl(fd, F_SETLK, &lock) == -1) {
                perror("Error unlocking account record");
            }

            close(fd);
            return balance;  // Success
        }
    }

    printf("Account with ID %d not found.\n", id);
    close(fd);
    return -1;  // Account not found
}


int apply_loan(int account_id, double loan_amt){
    Loan new_loan;
    new_loan.customer_ID = account_id;
    new_loan.employee_ID = -1;
    new_loan.loan_ID = generate_unique_loan_id();
    new_loan.loan_amount = loan_amt;
    new_loan.status = 0;
    save_loan_to_file(new_loan);
}

void customer_menu_test(int fd, User *customer) {
    int choice;
    double amount;
    int recipient_id;

    while (1) {
        printf("\nCustomer Menu:\n");
        printf("1. View Account Balance\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Transfer Funds\n");
        printf("5. Apply for a Loan\n");
        printf("6. Change Password\n");
        printf("7. Add Feedback\n");
        printf("8. View Transaction History\n");
        printf("9. Logout\n");
        printf("10. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                get_balance(customer->ID);
                
                break;
            
            case 2:
                printf("Enter amount to deposit: ");
                scanf("%lf", &amount);
                if (update_balance(customer->ID, amount, 1) == 0) {
                    printf("Deposit successful!\n");
                } else {
                    printf("Deposit failed!\n");
                }
                break;

            case 3:
                printf("Enter amount to withdraw: ");
                scanf("%lf", &amount);
                if (update_balance(customer->ID, -amount, 2) == 0) {
                    printf("Withdrawal successful!\n");
                } else {
                    printf("Insufficient balance or withdrawal failed!\n");
                }
                break;

            case 4:
                printf("Enter recipient customer ID: ");
                scanf("%d", &recipient_id);
                printf("Enter amount to transfer: ");
                scanf("%lf", &amount);
                if (transfer_funds(customer->ID, recipient_id, amount) == 0) {
                    printf("Transfer successful!\n");
                } else {
                    printf("Transfer failed!\n");
                }
                break;

            case 5:
                double loan_amt;
                printf("Enter loan Amount: ");
                scanf("%lf", &loan_amt);
                apply_loan(customer->ID, loan_amt);
                break;

            case 6:
                change_password(fd, customer);
                break;

            case 7:
                {
                    char feedback[256];
                    printf("Enter your feedback: ");
                    scanf(" %[^\n]s", feedback);
                    write_feedback(customer->ID, feedback);
                }
                break;

            case 8:
                view_transactions(customer->ID);
                break;

            case 9:
                printf("Logging out...\n");
                return;

            case 10:
                printf("Exiting...\n");
                exit(0);

            default:
                printf("Invalid choice, please try again.\n");
                break;
        }
    }
}

void customer_menu(int sockfd) {
    int choice;
    double amount;
    int recipient_id;
    char buffer[1024];

    while (1) {
        // Display the menu on the client side
        printf("\nCustomer Menu:\n");
        printf("1. View Account Balance\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Transfer Funds\n");
        printf("5. Apply for a Loan\n");
        printf("6. Change Password\n");
        printf("7. Add Feedback\n");
        printf("8. View Transaction History\n");
        printf("9. Logout\n");
        printf("10. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        memset(buffer, 0, sizeof(buffer));
        // Handle the menu choice
        switch (choice) {
            case 1:  // View balance
                snprintf(buffer, sizeof(buffer), "VIEW_BALANCE");
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Display balance
                break;

            case 2:  // Deposit money
                printf("Enter amount to deposit: ");
                scanf("%lf", &amount);
                snprintf(buffer, sizeof(buffer), "DEPOSIT %.2lf", amount);
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Success or failure message
                break;

            case 3:  // Withdraw money
                printf("Enter amount to withdraw: ");
                scanf("%lf", &amount);
                snprintf(buffer, sizeof(buffer), "WITHDRAW %.2lf", amount);
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Success or failure message
                break;

            case 4:  // Transfer funds
                printf("Enter recipient customer ID: ");
                scanf("%d", &recipient_id);
                printf("Enter amount to transfer: ");
                scanf("%lf", &amount);
                snprintf(buffer, sizeof(buffer), "TRANSFER %d %.2lf", recipient_id, amount);
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Success or failure message
                break;

            case 5:  // Apply for a loan
                printf("Enter loan amount: ");
                scanf("%lf", &amount);
                snprintf(buffer, sizeof(buffer), "APPLY_LOAN %.2lf",  amount);
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Loan application response
                break;

            case 6:{  // Change password
                printf("Enter your new password: ");
                getchar();  // To clear out the newline from previous input
                char newpassword[MAX_PASSWORD_LEN];
                scanf("%s", newpassword);
                snprintf(buffer, sizeof(buffer), "CHANGE_PASSWORD %s", newpassword);
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Password change response
                break;}

            case 7:  // Add feedback
                printf("Enter your feedback: ");
                getchar();  // To clear out the newline from previous input
                char feedback[256];
                fgets(feedback, sizeof(feedback), stdin);
                snprintf(buffer, sizeof(buffer), "ADD_FEEDBACK %s", feedback);
                printf("%s\n", buffer);
                write(sockfd, buffer, strlen(buffer));
                read(sockfd, buffer, sizeof(buffer));
                printf("%s\n", buffer);  // Feedback submission response
                break;

            case 8:  // View transaction history
                snprintf(buffer, sizeof(buffer), "VIEW_HISTORY");
                write(sockfd, buffer, strlen(buffer));
                //receive_transaction_history(sockfd);
                int n = read(sockfd, buffer, sizeof(buffer));
                printf("%s size %d\n", buffer, n);  // Transaction history
                break;

            case 9:  // Logout
                printf("Logging out...\n");
                snprintf(buffer, sizeof(buffer), "LOGOUT");
                write(sockfd, buffer, strlen(buffer));
                return;

            case 10:  // Exit
                printf("Exiting...\n");
                snprintf(buffer, sizeof(buffer), "EXIT");
                write(sockfd, buffer, strlen(buffer));
                exit(0);

            default:
                printf("Invalid choice, please try again.\n");
                break;
        }
    }
}

int handle_customer_request(int client_sock, User *customer, int fd) {
    char buffer[1024] = {0};
    int bytes_read;
    double amount;
    int recipient_id;

    while ((bytes_read = read(client_sock, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0';  // Null terminate the received message

        char command[256];
        sscanf(buffer, "%s", command);

        if (strcmp(command, "VIEW_BALANCE") == 0) {
            double balance = get_balance(customer->ID);
            snprintf(buffer, sizeof(buffer), "Your balance is: %.2lf", balance);
            write(client_sock, buffer, strlen(buffer));
        } 
        else if (strcmp(command, "DEPOSIT") == 0) {
            sscanf(buffer, "DEPOSIT %lf", &amount);
            if (update_balance(customer->ID, amount, 1) == 0) {
                snprintf(buffer, sizeof(buffer), "Deposit successful!");
            } else {
                snprintf(buffer, sizeof(buffer), "Deposit failed!");
            }
            write(client_sock, buffer, strlen(buffer));
        } 
        else if (strcmp(command, "WITHDRAW") == 0) {
            sscanf(buffer, "WITHDRAW %lf", &amount);
            if (update_balance(customer->ID, -amount, 2) == 0) {
                snprintf(buffer, sizeof(buffer), "Withdrawal successful!");
            } else {
                snprintf(buffer, sizeof(buffer), "Insufficient balance or withdrawal failed!");
            }
            write(client_sock, buffer, strlen(buffer));
        } 
        else if (strcmp(command, "TRANSFER") == 0) {
            sscanf(buffer, "TRANSFER %d %lf", &recipient_id, &amount);
            if (transfer_funds(customer->ID, recipient_id, amount) == 0) {
                snprintf(buffer, sizeof(buffer), "Transfer successful!");
            } else {
                snprintf(buffer, sizeof(buffer), "Transfer failed!");
            }
            write(client_sock, buffer, strlen(buffer));
        } 
        else if (strcmp(command, "APPLY_LOAN") == 0) {
            sscanf(buffer, "APPLY_LOAN %lf", &amount);
            apply_loan(customer->ID, amount);
            snprintf(buffer, sizeof(buffer), "Loan application submitted.");
            write(client_sock, buffer, strlen(buffer));
        }
        else if (strcmp(command, "CHANGE_PASSWORD") == 0) {
            char *newpassword;
            newpassword = buffer + sizeof("CHANGE_PASSWORD");
            strcpy(customer->password, newpassword);
            if(update_user(*customer) == 0){
                snprintf(buffer, sizeof(buffer), "Password changed!");
            }else{
                snprintf(buffer, sizeof(buffer), "Password change failed");
            }
            write(client_sock, buffer, strlen(buffer));
            
        } 
        else if (strcmp(command, "ADD_FEEDBACK") == 0) {
            char *feedback;
            //sscanf(buffer, "ADD_FEEDBACK %s", feedback);
            feedback = buffer + sizeof("add feedback");
            printf("%s\n", feedback);
            write_feedback(customer->ID, feedback);
            snprintf(buffer, sizeof(buffer), "Feedback submitted.");
            write(client_sock, buffer, strlen(buffer));
        }
        else if (strcmp(command, "VIEW_HISTORY") == 0) {
            memset(buffer, 0, sizeof(buffer));
            view_transaction_history(buffer, customer->ID);
            
            write(client_sock, buffer, strlen(buffer));
        }
        else if (strcmp(command, "LOGOUT") == 0) {
            logout(customer);
            return 0;
        }
        else if (strcmp(command, "EXIT") == 0) {
            printf("User exited.\n");
            logout(customer);
            close(client_sock);
            exit(0);
        }
    }
    return 1;
}