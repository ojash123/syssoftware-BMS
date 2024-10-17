#include "../headers/user.h"
#include "../headers/accounts.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void add_new_customer(int userfd) {
    User new_customer;
    printf("Enter new employee's username: ");
    scanf("%s", new_customer.username);
    printf("Enter password: ");
    scanf("%s", new_customer.password);
    printf("Enter ID: ");
    scanf("%d", &new_customer.ID);
    new_customer.role = 1; // Customer role
    Account useracc;
    useracc.user_ID = new_customer.ID;
    useracc.active = true;
    useracc.balance = 0;
    if(save_acc_to_file(useracc) != 0) return;
    lseek(userfd, 0, SEEK_END); // Move to the end of file to add a new record
    write(userfd, &new_customer, sizeof(User));
    
    printf("New employee added successfully.\n");
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

void employee_menu(int fd, User *employee){
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