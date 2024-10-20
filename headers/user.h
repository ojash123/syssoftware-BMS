#ifndef USER_H
#define USER_H
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define USER_ID_FILE "database/user_id_counter.dat"
#define USER_FILE "database/user.dat"
#include <semaphore.h>
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int role; // 1 = customer, 2 = employee, 3 = manager, 4 = admin
    int ID;
    sem_t user_semaphore;
} User;

void initialize_semaphore(User *user) ;
int login(User *user,  const char* username, const char* password);
int logout(User *user);
void change_password(int fd, User *user);
void modify_user(int fd, int userid) ;
int generate_unique_user_id();
User* read_user(int user_id);
void display_pending_loans();
int save_user_to_file(User user);
int update_user(User user);

#endif
