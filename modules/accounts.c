#include "../headers/user.h"
#include "../headers/accounts.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void update_balance(int customer_id, int fd, double amount){
    struct flock lock;
    Account account;

    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(Account);
    
    lseek(fd, 0, SEEK_SET);

    // Loop through the accounts in the file
    while (read(fd, &account, sizeof(Account)) > 0) {
        if (account.user_ID == customer_id) {
            if(!account.active){
                printf("This account is inactive\n");
                return -1;
            }
            if(account.balance + amount < 0){
                printf("This operation would reduce balance below 0\n");
                return -1;
            }
            lock.l_start = lseek(fd, -sizeof(Account), SEEK_CUR);
            lock.l_type = F_WRLCK;  // Apply write lock

            if (fcntl(fd, F_SETLKW, &lock) == -1) {
                perror("Error locking account");
                return -1;
            }
            
            account.balance += amount;

            // Write the updated account back to the file
            lseek(fd, -sizeof(Account), SEEK_CUR); // Move back to the record start
            if (write(fd, &account, sizeof(Account)) != sizeof(Account)) {
                perror("Error writing account");
                return -1;
            }

            // Unlock the record after updating
            lock.l_type = F_UNLCK;
            if (fcntl(fd, F_SETLK, &lock) == -1) {
                perror("Error unlocking account");
            }

            return 0;  // Success
        }
    }

    // Account not found
    return -1;
}

int generate_unique_loan_id() {
    int fd;
    int loan_id = 0;

    // Open or create the loan_id_counter.dat file with read/write access
    fd = open(LOAN_ID_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening loan ID file");
        return -1;
    }

    // Lock the file to avoid race conditions in a concurrent environment
    struct flock lock;
    lock.l_type = F_WRLCK; 
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = sizeof(int); 
    fcntl(fd, F_SETLKW, &lock);
    read(fd, &loan_id, sizeof(int));

    loan_id++;

    lseek(fd, 0, SEEK_SET);
    write(fd, &loan_id, sizeof(int));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);

    return loan_id;
}

Loan* read_loan(int loan_id) {
    int fd;
    Loan* loan = malloc(sizeof(Loan));
    struct flock lock;

    fd = open(LOAN_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening loan file");
        free(loan);
        return NULL;
    }

    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    // Read through the file to find the loan
    while (read(fd, loan, sizeof(Loan)) > 0) {
        if (loan->loan_ID == loan_id) {
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            return loan;
        }
    }

    // If loan not found
    printf("Loan ID %d not found.\n", loan_id);

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    free(loan);
    return NULL;
}
void display_pending_loans() {
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
        if (loan.status == 0) {
            printf("Loan ID: %d, Customer ID: %d, Amount: %.2f\n",
                   loan.loan_ID, loan.customer_ID, loan.loan_amount);
            if (loan.employee_ID == -1) {
                printf("Status: Unassigned\n");
            } else {
                printf("Assigned Employee ID: %d\n", loan.employee_ID);
            }
        }
    }

    close(fd);
}

int save_loan_to_file(Loan loan) {
    int fd;
    struct flock lock;

    // Open the loan file for writing or create it
    fd = open(LOAN_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening loan file");
        return -1;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    if (write(fd, &loan, sizeof(Loan)) == -1) {
        perror("Error writing loan to file");
        close(fd);
        return -1;
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return 0; // Success
}
int update_loan(Loan loan) {
    int fd;
    Loan temp_loan;
    struct flock lock;

    fd = open(LOAN_FILE, O_RDWR);
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
    while (read(fd, &temp_loan, sizeof(Loan)) > 0) {
        if (temp_loan.loan_ID == loan.loan_ID) {
            lseek(fd, -sizeof(Loan), SEEK_CUR);

            if (write(fd, &loan, sizeof(Loan)) == -1) {
                perror("Error updating loan");
                close(fd);
                return -1;
            }

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            return 0; // Success
        }
    }

    printf("Loan ID %d not found for update.\n", loan.loan_ID);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return -1; // Loan not found
}