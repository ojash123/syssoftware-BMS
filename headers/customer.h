#ifndef CUSTOMER_H
#define CUSTOMER_H
#include "accounts.h"
#include "user.h"

int write_feedback(int customer_id, const char* feedback) ;

int transfer_funds(int from_customer_id, int to_customer_id, double amount) ;

double get_balance(int id);
int apply_loan(int account_id, double amt);

void customer_menu(int sockfd);
int handle_customer_request(int client_sock, User *customer, int fd);

#endif