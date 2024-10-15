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