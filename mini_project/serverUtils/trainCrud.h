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

//---------------------- CRUD operation on train--------------------//

void crud_train(int client_sock)
{
    int valid = 0;
    int choice;
    read(client_sock, &choice, sizeof(choice));
    if (choice == 1)
    { // Add train
        char tname[50];
        int tid = 0;
        read(client_sock, &tname, sizeof(tname));
        struct train tdb, temp;
        struct flock lock;
        int fd_train = open("db/db_train.txt", O_RDWR);

        tdb.train_number = tid;
        strcpy(tdb.train_name, tname);
        tdb.total_seats = 10; // by default, we are taking 10 seats
        tdb.available_seats = 10;

        int fp = lseek(fd_train, 0, SEEK_END);

        lock.l_type = F_WRLCK;
        lock.l_start = fp;
        lock.l_len = 0;
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lock);

        if (fp == 0)
        {
            // no trains present initially
            valid = 1;
            write(fd_train, &tdb, sizeof(tdb));
            lock.l_type = F_UNLCK;
            fcntl(fd_train, F_SETLK, &lock);
            close(fd_train);
            write(client_sock, &valid, sizeof(valid));
        }
        else
        {
            valid = 1;
            lseek(fd_train, -1 * sizeof(struct train), SEEK_END);
            read(fd_train, &temp, sizeof(temp));
            tdb.train_number = temp.train_number + 1;
            write(fd_train, &tdb, sizeof(tdb));
            write(client_sock, &valid, sizeof(valid));
        }
        lock.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lock);
        close(fd_train);
    }

    else if (choice == 2)
    { // View/ Read trains
        struct flock lock;
        struct train tdb;
        int fd_train = open("db/db_train.txt", O_RDONLY);

        lock.l_type = F_RDLCK;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lock);
        int fp = lseek(fd_train, 0, SEEK_END);
        int no_of_trains = fp / sizeof(struct train);
        write(client_sock, &no_of_trains, sizeof(int));

        lseek(fd_train, 0, SEEK_SET);
        while (fp != lseek(fd_train, 0, SEEK_CUR))
        {
            read(fd_train, &tdb, sizeof(tdb));
            write(client_sock, &tdb.train_number, sizeof(int));
            write(client_sock, &tdb.train_name, sizeof(tdb.train_name));
            write(client_sock, &tdb.total_seats, sizeof(int));
            write(client_sock, &tdb.available_seats, sizeof(int));
        }
        valid = 1;
        lock.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lock);
        close(fd_train);
    }

    else if (choice == 3)
    { // Update train
        crud_train(client_sock);
        int choice, valid = 0, tid;
        struct flock lock;
        struct train tdb;
        int fd_train = open("db/db_train.txt", O_RDWR);

        read(client_sock, &tid, sizeof(tid));

        lock.l_type = F_WRLCK;
        lock.l_start = (tid) * sizeof(struct train);
        lock.l_len = sizeof(struct train);
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lock);

        lseek(fd_train, 0, SEEK_SET);
        lseek(fd_train, (tid) * sizeof(struct train), SEEK_CUR);
        read(fd_train, &tdb, sizeof(struct train));

        read(client_sock, &choice, sizeof(int));
        if (choice == 1)
        { // update train name
            write(client_sock, &tdb.train_name, sizeof(tdb.train_name));
            read(client_sock, &tdb.train_name, sizeof(tdb.train_name));
        }
        else if (choice == 2)
        { // update total number of seats
            write(client_sock, &tdb.total_seats, sizeof(tdb.total_seats));
            read(client_sock, &tdb.total_seats, sizeof(tdb.total_seats));
        }

        lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);
        write(fd_train, &tdb, sizeof(struct train));
        valid = 1;
        write(client_sock, &valid, sizeof(valid));
        lock.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lock);
        close(fd_train);
    }

    else if (choice == 4)
    { // Delete train
        crud_train(client_sock);
        struct flock lock;
        struct train tdb;
        int fd_train = open("db/db_train.txt", O_RDWR);
        int tid, valid = 0;

        read(client_sock, &tid, sizeof(tid));

        lock.l_type = F_WRLCK;
        lock.l_start = (tid) * sizeof(struct train);
        lock.l_len = sizeof(struct train);
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lock);

        lseek(fd_train, 0, SEEK_SET);
        lseek(fd_train, (tid) * sizeof(struct train), SEEK_CUR);
        read(fd_train, &tdb, sizeof(struct train));
        strcpy(tdb.train_name, "deleted");
        lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);
        write(fd_train, &tdb, sizeof(struct train));
        valid = 1;
        write(client_sock, &valid, sizeof(valid));
        lock.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lock);
        close(fd_train);
    }
}
