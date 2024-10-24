#ifndef ACCOUNTS_H
#define ACCOUNTS_H
#define LOAN_ID_FILE "database/loan_id_counter.dat"
#define LOAN_FILE "database/loans.dat"
#define ACCOUNT_FILE "database/accounts.dat"
#define TRANSACTION_FILE "database/transactions.dat"
#define FEEDBACK_FILE "database/feedback.txt"
#include "user.h"
#include <stdbool.h>
typedef struct {
    int user_ID;
    bool active;
    double balance;

} Account;
typedef struct {
    int user_ID;
    int type; //1 - deposit, 2 - withdraw, 3 - transfer from, 4 - transfer to, 5 - loan 
    double amt;

} Transaction;
typedef struct {
    int loan_ID;          
    int customer_ID;       
    int employee_ID;      
    double loan_amount;   
    int status;     // Current status of the loan 0 - pending 1 approved, -1 rejected
} Loan;
int update_balance(int customer_id,  double amount, int type);
int save_acc_to_file(Account acc);
int save_transaction(Transaction t);
void view_transactions(int customer_id);
void receive_transaction_history(int sock_fd);
void view_transaction_history(char* buffer, int customer_id);
int generate_unique_loan_id();
Loan* read_loan(int loan_id);
void display_pending_loans();
int save_loan_to_file(Loan loan);
int update_loan(Loan loan);
#endif