#include "../headers/user.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <fcntl.h>
#include <stdlib.h>

int login(User *user, int fd, const char* username, const char* password){
    struct flock lock;
      
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(User);
    lock.l_type = F_WRLCK;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, user, sizeof(User)) > 0 ){
        if(strcmp(user->username, username) == 0 && strcmp(user->password, password) == 0){
            struct flock test_lock = lock;
            lock.l_start = lseek(fd, -sizeof(User), SEEK_CUR); //Set the lock start to current record
            test_lock = lock;
            if (fcntl(fd, F_GETLK, &test_lock) == -1) {
                perror("Error checking lock");
            }
            if(test_lock.l_type == F_UNLCK){
                //lock.l_type = F_WRLCK;
                if (fcntl(fd, F_SETLK, &lock) == -1) {
                    perror("Error locking file");
                }
                user->user_lock = lock;
                printf("user %s logged in\n", user->username);
                return 0; //login success
            }else{
                printf("User already logged in!\n");
                return -1;
            }
            
        }
    }
    return -1; // Login unsuccessful
}
int logout(User *user, int fd){
    if(user == NULL){
        return -1;
    }
    user->user_lock.l_type = F_UNLCK;
    if(fcntl(fd, F_SETLK, &user->user_lock) == -1){
        perror("Error unlocking");
        return -1;
    }
    printf("User %s successfully logged out\n", user->username);
    return 0; //logout success
}

void change_password(int fd, User *user){
    User temp_user;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &temp_user, sizeof(User)) > 0) {
        if (strcmp(temp_user.username, user->username) == 0) {
            printf("Enter new password for %s: ", user->username);
            scanf("%s", temp_user.password);
            lseek(fd, -sizeof(User), SEEK_CUR);
            write(fd, &temp_user, sizeof(User));
            printf("User details updated.\n");
            return;
        }
    }
    printf("User not found.\n");
}