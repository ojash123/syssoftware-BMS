#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include "../headers/user.h"

int add_new_customer(User new_customer) ;

void view_assigned_loans(int employee_id);
int process_loan(int loan_id, int approve);

void employee_menu(int fd);
int handle_employee_request(int client_sock, User *employee);

#endif