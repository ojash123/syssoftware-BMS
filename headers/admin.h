#ifndef ADMIN_H
#define ADMIN_H
#include "../headers/user.h"
void add_new_employee(int fd);
void modify_user(int fd, const char *username);
void modify_role(int fd, const char *username, int new_role);
void change_password(int fd, User *admin_user);
void admin_menu(int fd, User *admin_user);

#endif