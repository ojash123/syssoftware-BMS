#ifndef ADMIN_H
#define ADMIN_H
#include "../headers/user.h"
int add_new_employee(User new_employee);
int modify_role(int user_id, int new_role);

void admin_menu(int fd);
int handle_admin_request(int client_sock, User *admin) ;

#endif