#ifndef USER_H
#define USER_H
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define BUFFER_SIZE 200
#include <fcntl.h>
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int role; // 1 = customer, 2 = employee, 3 = manager, 4 = admin
    int ID;
    struct flock user_lock;
} User;

int login(User *user, int fd, const char* username, const char* password);
int logout(User *user, int fd);
void change_password(int fd, User *user);
void modify_user(int fd, int userid) ;
#endif
