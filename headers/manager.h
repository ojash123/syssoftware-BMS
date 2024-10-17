#ifndef MANAGER_H
#define MANAGER_H
#include "accounts.h"
#include "user.h"

int activate_account(int cust_id, bool active);

int assign_loan_to_employee(int loan_id, int emp_id);

void display_all_feedback() ;

void manager_menu(int fd, User *manager);

#endif