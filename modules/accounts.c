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