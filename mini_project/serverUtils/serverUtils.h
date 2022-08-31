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
#include "struc.h"
#include "trainCrud.h"
#include "userCrud.h"

//------------------------------Function declarations---------------------------------------//

void service_cli(int sock);
void login(int client_sock);
void signup(int client_sock);
int menu(int client_sock, int type, int id);
void crud_train(int client_sock);
void crud_user(int client_sock);
int user_function(int client_sock, int choice, int type, int id);

//-------------------- Service every client-----------------------------//
void service_cli(int sock)
{
    int choice;
    printf("\n\tClient [%d] Connected\n", sock);
    do
    {
        read(sock, &choice, sizeof(int));
        if (choice == 1)
            login(sock);
        if (choice == 2)
            signup(sock);
        if (choice == 3)
            break;
    } while (1);

    close(sock);
    printf("\n\tClient [%d] Disconnected\n", sock);
}

//-------------------- Login function-----------------------------//

void login(int client_sock)
{
    int fd_user = open("db/db_user.txt", O_RDWR);
    int id, type, valid = 1, user_valid = 0;
    char password[50];
    struct user u;
    read(client_sock, &id, sizeof(id));
    read(client_sock, &password, sizeof(password));

    struct flock lock;

    lock.l_start = (id - 1) * sizeof(struct user);
    lock.l_len = sizeof(struct user);
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    lock.l_type = F_WRLCK;
    fcntl(fd_user, F_SETLKW, &lock);

    while (read(fd_user, &u, sizeof(u)))
    {
        if (u.login_id == id)
        {
            printf("User entry present in db\n");
          
            user_valid = 1;
            if (strncmp(password, u.password, strlen(password)) == 0)
            {
                printf("User password is correct\n");
                valid = 1;
                type = u.type;
                break;
            }
            else
            {
                valid = 0;
                break;
            }
        }
        else
        {
            user_valid = 0;
            valid = 0;
        }
    }

    // same agent is allowed from multiple terminals..
    // so unlocking his user record just after checking his credentials and allowing further
    if (type != 2)
    {
        lock.l_type = F_UNLCK;
        fcntl(fd_user, F_SETLK, &lock);
        close(fd_user);
    }

    // if valid user, show him menu
    if (user_valid)
    {
        write(client_sock, &valid, sizeof(valid));
        if (valid)
        {
            //write the type of user in client_socket, so client side identifies which menu to show[admin or customer/agent]
            write(client_sock, &type, sizeof(type));
            while (menu(client_sock, type, id) != -1)
                ;
        }
    }
    else
        write(client_sock, &valid, sizeof(valid)); //pas the valid=0 value in socket to tell client its invalid

    // same user is not allowed from multiple terminals..
    // so unlocking his user record after he logs out only to not allow him from other terminal
    if (type == 2)
    { //customer
        lock.l_type = F_UNLCK;
        fcntl(fd_user, F_SETLK, &lock);
        close(fd_user);
    }
}

//-------------------- Signup function-----------------------------//

void signup(int client_sock)
{
    int fd_user = open("db/db_user.txt", O_RDWR);
    int type, id = 0;
    char name[50], password[50];
    struct user u, temp;

    read(client_sock, &type, sizeof(type));
    read(client_sock, &name, sizeof(name));
    read(client_sock, &password, sizeof(password));

    int fp = lseek(fd_user, 0, SEEK_END);

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = fp;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    lock.l_pid = getpid();

    fcntl(fd_user, F_SETLKW, &lock);

    // if file is empty, login id will start from 1
    // else it will increment from the previous value
    if (fp == 0)
    {
        u.login_id = 1;
        strcpy(u.name, name);
        strcpy(u.password, password);
        u.type = type;
        write(fd_user, &u, sizeof(u));
        write(client_sock, &u.login_id, sizeof(u.login_id));
    }
    else
    {
        fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
        read(fd_user, &u, sizeof(u));
        u.login_id++;
        strcpy(u.name, name);
        strcpy(u.password, password);
        u.type = type;
        write(fd_user, &u, sizeof(u));
        write(client_sock, &u.login_id, sizeof(u.login_id));
    }
    lock.l_type = F_UNLCK;
    fcntl(fd_user, F_SETLK, &lock);

    close(fd_user);
}

//-------------------- Main menu function-----------------------------//

int menu(int client_sock, int type, int id)
{
    int choice, ret;

    // for admin
    if (type == 0)
    {
        read(client_sock, &choice, sizeof(choice));
        if (choice == 1)
        { // CRUD options on train
            crud_train(client_sock);
            return menu(client_sock, type, id);
        }
        else if (choice == 2)
        { // CRUD options on User
            crud_user(client_sock);
            return menu(client_sock, type, id);
        }
        else if (choice == 3) // Logout
            return -1;
    }
    else if (type == 2 || type == 1)
    { // For agent and customer
        read(client_sock, &choice, sizeof(choice));
        printf("User chose choice: %d\n", choice);
        ret = user_function(client_sock, choice, type, id);
        if (ret != 5)
            return menu(client_sock, type, id);
        else if (ret == 5)
            return -1;
    }
}

//---------------------- User functions -----------------------//

int user_function(int client_sock, int choice, int type, int id)
{
    int valid = 0;
    if (choice == 1)
    { // book ticket
        printf("\ninside user_function\n");
        crud_train(client_sock);
        struct flock lockt;
        struct flock lockb;
        struct train tdb;
        struct booking bdb;
        int fd_train = open("db/db_train.txt", O_RDWR);
        int fd_book = open("db/db_booking.txt", O_RDWR);
        int tid, seats;
        read(client_sock, &tid, sizeof(tid));

        lockt.l_type = F_WRLCK;
        lockt.l_start = tid * sizeof(struct train);
        lockt.l_len = sizeof(struct train);
        lockt.l_whence = SEEK_SET;
        lockt.l_pid = getpid();

        lockb.l_type = F_WRLCK;
        lockb.l_start = 0;
        lockb.l_len = 0;
        lockb.l_whence = SEEK_END;
        lockb.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lockt);
        lseek(fd_train, tid * sizeof(struct train), SEEK_SET);

        read(fd_train, &tdb, sizeof(tdb));
        read(client_sock, &seats, sizeof(seats));

        if (tdb.train_number == tid)
        {
            if (tdb.available_seats >= seats)
            {
                valid = 1;
                tdb.available_seats -= seats;
                fcntl(fd_book, F_SETLKW, &lockb);
                int fp = lseek(fd_book, 0, SEEK_END);

                if (fp > 0)
                {
                    lseek(fd_book, -1 * sizeof(struct booking), SEEK_CUR);
                    read(fd_book, &bdb, sizeof(struct booking));
                    bdb.booking_id++;
                }
                else
                    bdb.booking_id = 0;

                bdb.type = type;
                bdb.uid = id;
                bdb.tid = tid;
                bdb.seats = seats;
                write(fd_book, &bdb, sizeof(struct booking));
                lockb.l_type = F_UNLCK;
                fcntl(fd_book, F_SETLK, &lockb);
                close(fd_book);
            }

            lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);
            write(fd_train, &tdb, sizeof(tdb));
        }

        lockt.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lockt);
        close(fd_train);
        write(client_sock, &valid, sizeof(valid));
        return valid;
    }

    else if (choice == 2)
    { // View bookings
        struct flock lock;
        struct booking bdb;
        int fd_book = open("db/db_booking.txt", O_RDONLY);
        int no_of_bookings = 0;

        lock.l_type = F_RDLCK;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_whence = SEEK_SET;
        lock.l_pid = getpid();

        fcntl(fd_book, F_SETLKW, &lock);

        while (read(fd_book, &bdb, sizeof(bdb)))
        {
            if (bdb.uid == id)
                no_of_bookings++;
        }

        write(client_sock, &no_of_bookings, sizeof(int));
        lseek(fd_book, 0, SEEK_SET);

        while (read(fd_book, &bdb, sizeof(bdb)))
        {
            if (bdb.uid == id)
            {
                write(client_sock, &bdb.booking_id, sizeof(int));
                write(client_sock, &bdb.tid, sizeof(int));
                write(client_sock, &bdb.seats, sizeof(int));
            }
        }
        lock.l_type = F_UNLCK;
        fcntl(fd_book, F_SETLK, &lock);
        close(fd_book);
        return valid;
    }

    else if (choice == 3)
    { // update booking
        int choice = 2, bid, val;
        user_function(client_sock, choice, type, id);
        struct booking bdb;
        struct train tdb;
        struct flock lockb;
        struct flock lockt;
        int fd_book = open("db/db_booking.txt", O_RDWR);
        int fd_train = open("db/db_train.txt", O_RDWR);
        read(client_sock, &bid, sizeof(bid));

        lockb.l_type = F_WRLCK;
        lockb.l_start = bid * sizeof(struct booking);
        lockb.l_len = sizeof(struct booking);
        lockb.l_whence = SEEK_SET;
        lockb.l_pid = getpid();

        fcntl(fd_book, F_SETLKW, &lockb);
        lseek(fd_book, bid * sizeof(struct booking), SEEK_SET);
        read(fd_book, &bdb, sizeof(bdb));
        lseek(fd_book, -1 * sizeof(struct booking), SEEK_CUR);

        lockt.l_type = F_WRLCK;
        lockt.l_start = (bdb.tid) * sizeof(struct train);
        lockt.l_len = sizeof(struct train);
        lockt.l_whence = SEEK_SET;
        lockt.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lockt);
        lseek(fd_train, (bdb.tid) * sizeof(struct train), SEEK_SET);
        read(fd_train, &tdb, sizeof(tdb));
        lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);

        read(client_sock, &choice, sizeof(choice));

        if (choice == 1)
        { // increase number of seats required of booking id
            read(client_sock, &val, sizeof(val));
            if (tdb.available_seats >= val)
            {
                valid = 1;
                tdb.available_seats -= val;
                bdb.seats += val;
            }
        }
        else if (choice == 2)
        { // decrease number of seats required of booking id
            valid = 1;
            read(client_sock, &val, sizeof(val));
            tdb.available_seats += val;
            bdb.seats -= val;
        }

        write(fd_train, &tdb, sizeof(tdb));
        lockt.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lockt);
        close(fd_train);

        write(fd_book, &bdb, sizeof(bdb));
        lockb.l_type = F_UNLCK;
        fcntl(fd_book, F_SETLK, &lockb);
        close(fd_book);

        write(client_sock, &valid, sizeof(valid));
        return valid;
    }
    else if (choice == 4)
    { // Cancel an entire booking
        int choice = 2, bid;
        user_function(client_sock, choice, type, id);
        struct booking bdb;
        struct train tdb;
        struct flock lockb;
        struct flock lockt;
        int fd_book = open("db/db_booking.txt", O_RDWR);
        int fd_train = open("db/db_train.txt", O_RDWR);
        read(client_sock, &bid, sizeof(bid));

        lockb.l_type = F_WRLCK;
        lockb.l_start = bid * sizeof(struct booking);
        lockb.l_len = sizeof(struct booking);
        lockb.l_whence = SEEK_SET;
        lockb.l_pid = getpid();

        fcntl(fd_book, F_SETLKW, &lockb);
        lseek(fd_book, bid * sizeof(struct booking), SEEK_SET);
        read(fd_book, &bdb, sizeof(bdb));
        lseek(fd_book, -1 * sizeof(struct booking), SEEK_CUR);

        lockt.l_type = F_WRLCK;
        lockt.l_start = (bdb.tid) * sizeof(struct train);
        lockt.l_len = sizeof(struct train);
        lockt.l_whence = SEEK_SET;
        lockt.l_pid = getpid();

        fcntl(fd_train, F_SETLKW, &lockt);
        lseek(fd_train, (bdb.tid) * sizeof(struct train), SEEK_SET);
        read(fd_train, &tdb, sizeof(tdb));
        lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);

        tdb.available_seats += bdb.seats;
        bdb.seats = 0;
        valid = 1;

        write(fd_train, &tdb, sizeof(tdb));
        lockt.l_type = F_UNLCK;
        fcntl(fd_train, F_SETLK, &lockt);
        close(fd_train);

        write(fd_book, &bdb, sizeof(bdb));
        lockb.l_type = F_UNLCK;
        fcntl(fd_book, F_SETLK, &lockb);
        close(fd_book);

        write(client_sock, &valid, sizeof(valid));
        return valid;
    }
    else if (choice == 5) // Logout
        return 5;
}
