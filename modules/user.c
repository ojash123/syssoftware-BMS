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

    lseek(fd, 0, SEEK_SET);
    while(read(fd, user, sizeof(User)) > 0 ){
        if(strcmp(user->username, username) == 0 && strcmp(user->password, password) == 0){
            lock.l_start = lseek(fd, -sizeof(User), SEEK_CUR); //Set the lock start to current record
            lock.l_type = F_WRLCK; //try to acquire a exclusive lock
            if (fcntl(fd, F_GETLK, &lock) == -1) {
                perror("Error checking lock");
            }
            if(lock.l_type == F_UNLCK){
                lock.l_type = F_WRLCK;
                if (fcntl(fd, F_SETLK, &lock) == -1) {
                    perror("Error locking file");
                }
                user->user_lock = lock;
                return 0; //login success
            }else{
                printf("User logged in!\n");
                return -1;
            }
            
        }
    }
    return -1; // Login unsuccessful
}
void logout(User *user){

}