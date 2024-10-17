#include "../headers/user.h"
#include "../headers/accounts.h"
#include "../headers/manager.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int activate_account(int cust_id, bool active){
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
            temp_account.active = active;
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


void display_all_feedback() {
    int fd = open(FEEDBACK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening feedback file");
        return;
    }

    char buffer[256];
    ssize_t bytes_read;

    printf("---- All Customer Feedback ----\n");

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the buffer for safe printing
        printf("%s", buffer);
    }

    if (bytes_read == -1) {
        perror("Error reading feedback file");
    }

    close(fd);  
}

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
