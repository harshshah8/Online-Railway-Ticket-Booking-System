/*
Author: Harsh Shah
Roll: MT2021050
*/

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//---------------------- CRUD operation on user--------------------//
void crud_user(int client_sock)
{
    int valid = 0;
    int choice;
    read(client_sock, &choice, sizeof(choice));
    if (choice == 1)
    { // Add user
        char name[50], password[50];
        int type;
        read(client_sock, &type, sizeof(type));
        read(client_sock, &name, sizeof(name));
        read(client_sock, &password, sizeof(password));

        struct user udb;
        struct flock lock;
        int fd_user = open("db/db_user.txt", O_RDWR);
        int fp = lseek(fd_user, 0, SEEK_END);

        lock.l_type = F_WRLCK;
        lock.l_start = fp;
        lock.l_len = 0;
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_user, F_SETLKW, &lock);

        if (fp == 0)
        {
            udb.login_id = 1;
            strcpy(udb.name, name);
            strcpy(udb.password, password);
            udb.type = type;
            write(fd_user, &udb, sizeof(udb));
            valid = 1;
            write(client_sock, &valid, sizeof(int));
            write(client_sock, &udb.login_id, sizeof(udb.login_id));
        }
        else
        {
            fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
            read(fd_user, &udb, sizeof(udb));
            udb.login_id++;
            strcpy(udb.name, name);
            strcpy(udb.password, password);
            udb.type = type;
            write(fd_user, &udb, sizeof(udb));
            valid = 1;
            write(client_sock, &valid, sizeof(int));
            write(client_sock, &udb.login_id, sizeof(udb.login_id));
        }
        lock.l_type = F_UNLCK;
        fcntl(fd_user, F_SETLK, &lock);
        close(fd_user);
    }

    else if (choice == 2)
    { // View user list
        struct flock lock;
        struct user udb;
        int fd_user = open("db/db_user.txt", O_RDONLY);

        lock.l_type = F_RDLCK;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_user, F_SETLKW, &lock);
        int fp = lseek(fd_user, 0, SEEK_END);
        int no_of_users = fp / sizeof(struct user);
        no_of_users--;
        write(client_sock, &no_of_users, sizeof(int));

        lseek(fd_user, 0, SEEK_SET);
        while (fp != lseek(fd_user, 0, SEEK_CUR))
        {
            read(fd_user, &udb, sizeof(udb));
            if (udb.type != 0)
            {
                write(client_sock, &udb.login_id, sizeof(int));
                write(client_sock, &udb.name, sizeof(udb.name));
                write(client_sock, &udb.type, sizeof(int));
            }
        }
        valid = 1;
        lock.l_type = F_UNLCK;
        fcntl(fd_user, F_SETLK, &lock);
        close(fd_user);
    }

    else if (choice == 3)
    { // Update user
        crud_user(client_sock);
        int choice, valid = 0, uid;
        char pass[50];
        struct flock lock;
        struct user udb;
        int fd_user = open("db/db_user.txt", O_RDWR);

        read(client_sock, &uid, sizeof(uid));

        lock.l_type = F_WRLCK;
        lock.l_start = (uid - 1) * sizeof(struct user);
        lock.l_len = sizeof(struct user);
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_user, F_SETLKW, &lock);

        lseek(fd_user, 0, SEEK_SET);
        lseek(fd_user, (uid - 1) * sizeof(struct user), SEEK_CUR);
        read(fd_user, &udb, sizeof(struct user));

        read(client_sock, &choice, sizeof(int));
        if (choice == 1)
        { // update name
            write(client_sock, &udb.name, sizeof(udb.name));
            read(client_sock, &udb.name, sizeof(udb.name));
            valid = 1;
            write(client_sock, &valid, sizeof(valid));
        }
        else if (choice == 2)
        { // update password
            read(client_sock, &pass, sizeof(pass));
            if (strncmp(pass, udb.password, strlen(pass)) == 0)
                valid = 1;
            write(client_sock, &valid, sizeof(valid));
            read(client_sock, &udb.password, sizeof(udb.password));
        }

        lseek(fd_user, -1 * sizeof(struct user), SEEK_CUR);
        write(fd_user, &udb, sizeof(struct user));
        if (valid)
            write(client_sock, &valid, sizeof(valid));
        lock.l_type = F_UNLCK;
        fcntl(fd_user, F_SETLK, &lock);
        close(fd_user);
    }

    else if (choice == 4)
    { // Delete any particular user
        crud_user(client_sock);
        struct flock lock;
        struct user udb;
        int fd_user = open("db/db_user.txt", O_RDWR);
        int uid, valid = 0;

        read(client_sock, &uid, sizeof(uid));

        lock.l_type = F_WRLCK;
        lock.l_start = (uid - 1) * sizeof(struct user);
        lock.l_len = sizeof(struct user);
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_user, F_SETLKW, &lock);

        lseek(fd_user, 0, SEEK_SET);
        lseek(fd_user, (uid - 1) * sizeof(struct user), SEEK_CUR);
        read(fd_user, &udb, sizeof(struct user));
        strcpy(udb.name, "deleted");
        strcpy(udb.password, "");
        lseek(fd_user, -1 * sizeof(struct user), SEEK_CUR);
        write(fd_user, &udb, sizeof(struct user));
        valid = 1;
        write(client_sock, &valid, sizeof(valid));
        lock.l_type = F_UNLCK;
        fcntl(fd_user, F_SETLK, &lock);
        close(fd_user);
    }
}
