#ifndef MANAGER_H
#define MANAGER_H
#include "accounts.h"
#include "user.h"

int activate_account(int cust_id);

int assign_loan_to_employee(int loan_id, int emp_id);
void get_pending_loans(char *buffer, size_t buffer_size) ;
void read_all_feedback(char *feedback) ;

void manager_menu(int fd);
int handle_manager_request(int client_sock, User *manager);

#endif