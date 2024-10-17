#include "../headers/user.h"
#include "../headers/accounts.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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

double get_balance(int account_id);

int apply_loan(int account_id);

void customer_menu(int fd, User *customer) {
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
                double balance = get_balance(customer->ID);
                printf("Your current balance: %.2f\n", balance);
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
                apply_loan(customer->ID);
                break;

            case 6:
                change_password(fd, customer);
                break;

            case 7:
                {
                    char feedback[256];
                    printf("Enter your feedback: ");
                    scanf(" %[^\n]s", feedback);
                    add_feedback(customer->ID, feedback);
                }
                break;

            case 8:
                view_transaction_history(customer->ID);
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