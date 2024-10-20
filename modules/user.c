#include "../headers/user.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
void initialize_semaphore(User *user) {
    // Initialize the semaphore for the user
    if (sem_init(&user->user_semaphore, 1, 1) == -1) { // 1 indicates shared between processes
        perror("Error initializing semaphore");
        exit(EXIT_FAILURE);
    }
}


int login(User *user, const char* username, const char* password){
    
    int fd;
    struct flock lock;

    fd = open(USER_FILE, O_RDWR);
    if (fd == -1) {
        perror("Error opening user file");
        free(user);
        return -1;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);
    while(read(fd, user, sizeof(User)) > 0 ){
        if(strcmp(user->username, username) == 0 && strcmp(user->password, password) == 0){
            if (sem_trywait(&user->user_semaphore) == 0) {
                // Semaphore acquired
                lseek(fd, -sizeof(User), SEEK_CUR);
                write(fd, user, sizeof(User));
                printf("User %s logged in\n", user->username);
                return 0; // Login success
            } else {
                // Semaphore already locked
                printf("User already logged in!\n");
                return -1; // Login failed
            }
        }
    }
    return -1; // Login unsuccessful
}
int logout(User *user){
    if(user == NULL){
        return -1;
    }
    sem_post(&user->user_semaphore);
    update_user(*user);
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

void modify_user(int fd, int userid) {
    User temp_user;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &temp_user, sizeof(User)) > 0) {
        if (userid == temp_user.ID) {
            printf("Enter new username for %s: ", temp_user.username);
            scanf("%s", temp_user.username);
            printf("Enter new password for %s: ", temp_user.username);
            scanf("%s", temp_user.password);
            printf("Enter ID: ");
            scanf("%d", &temp_user.ID);
            lseek(fd, -sizeof(User), SEEK_CUR);
            write(fd, &temp_user, sizeof(User));
            printf("User details updated.\n");
            return;
        }
    }
    printf("User not found.\n");
}

int generate_unique_user_id() {
    int fd;
    int user_id = 0;

    // Open or create the user_id_counter.dat file with read/write access
    fd = open(USER_ID_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening user ID file");
        return -1;
    }

    // Lock the file to avoid race conditions in a concurrent environment
    struct flock lock;
    lock.l_type = F_WRLCK; 
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = sizeof(int); 
    fcntl(fd, F_SETLKW, &lock);
    read(fd, &user_id, sizeof(int));

    user_id++;

    lseek(fd, 0, SEEK_SET);
    write(fd, &user_id, sizeof(int));

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);

    return user_id;
}

User* read_user(int user_id) {
    int fd;
    User* user = malloc(sizeof(User));
    struct flock lock;

    fd = open(USER_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening user file");
        free(user);
        return NULL;
    }

    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    // Read through the file to find the user
    while (read(fd, user, sizeof(User)) > 0) {
        if (user->ID == user_id) {
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            return user;
        }
    }

    // If user not found
    printf("user ID %d not found.\n", user_id);

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    free(user);
    return NULL;
}

int save_user_to_file(User user) {
    int fd;
    struct flock lock;

    // Open the user file for writing or create it
    fd = open(USER_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening user file");
        return -1;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    if (write(fd, &user, sizeof(user)) == -1) {
        perror("Error writing user to file");
        close(fd);
        return -1;
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return 0; // Success
}
int update_user(User user) {
    int fd;
    User temp_user;
    struct flock lock;

    fd = open(USER_FILE, O_RDWR);
    if (fd == -1) {
        perror("Error opening user file");
        return -1;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd, F_SETLKW, &lock);

    lseek(fd, 0, SEEK_SET);
    while (read(fd, &temp_user, sizeof(user)) > 0) {
        if (temp_user.ID == user.ID) {
            lseek(fd, -sizeof(user), SEEK_CUR);

            if (write(fd, &user, sizeof(user)) == -1) {
                perror("Error updating user");
                close(fd);
                return -1;
            }

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            return 0; // Success
        }
    }

    printf("user ID %d not found for update.\n", user.ID);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return -1; // user not found
}