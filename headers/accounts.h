#ifndef ACCOUNTS_H
#define ACCOUNTS_H
#define LOAN_ID_FILE "../loan_id_counter.dat"
#include "user.h"
typedef struct {
    int user_ID;
    bool active;
    double balance;

} Account;
typedef struct {
    int loan_ID;          
    int customer_ID;       
    int employee_ID;      
    double loan_amount;   
    int status;     // Current status of the loan 0 - pending 1 approved, -1 rejected
} Loan;
void update_balance(int customer_id, int accfd, double amount);
int generate_unique_loan_id();
#endif