#ifndef ADMIN_H
#define ADMIN_H
#include "../headers/user.h"
void add_new_employee(int fd);
void modify_role(int fd, const char *username, int new_role);

void admin_menu(int fd, User *admin_user);

#endif