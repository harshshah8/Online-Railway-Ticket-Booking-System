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
#include "userCrud.h"
#include "trainCrud.h"

//------------------------------Function declarations---------------------------------------//

int client(int sock);
int menu(int sock, int type);
int user_function(int sock, int choice);
int crud_train(int sock, int choice);
int crud_user(int sock, int choice);

//-------------------- First function which is called-----------------------------//

int client(int sock)
{
    int choice, valid;
    //system("clear");
    printf("\n\n\t\t\tTRAIN RESERVATION SYSTEM\n\n");
    printf("\t1. Sign In\n");
    printf("\t2. Sign Up\n");
    printf("\t3. Exit\n");
    printf("\tEnter Your Choice:: ");
    scanf("%d", &choice);
    write(sock, &choice, sizeof(choice));
    if (choice == 1)
    { // Log in
        int id, type;
        char password[50];
        printf("\tlogin id:: ");
        scanf("%d", &id);
        printf("\tpassword:: ");
        scanf("%s", password);
        write(sock, &id, sizeof(id));
        write(sock, &password, sizeof(password));
        read(sock, &valid, sizeof(valid));
        if (valid)
        {
            // printf("Valid flag is set as 1 and recvd successfully\n");
            read(sock, &type, sizeof(type));
            printf("\tlogin successfull for client of type: %d\n", type);
            while (menu(sock, type) != -1)
                ;
            //system("clear");
            return 1;
        }
        else
        {
            // printf("Valid flag is set as %d and recvd successfully\n", valid);
            printf("\tLogin Failed : Incorrect password or login id\n");
            return 1;
        }
    }

    else if (choice == 2)
    { // Sign up
        int type, id;
        char name[50], password[50], secret_pin[6];
        system("clear");
        printf("\n\tEnter The Type Of Account:: \n");
        printf("\t0. Admin\n\t1. Agent\n\t2. Customer\n");
        printf("\tYour Response: ");
        scanf("%d", &type);
        printf("\tEnter Your Name: ");
        scanf("%s", name);
        printf("\tEnter your password: ");
        scanf("%s", password);

        if (type == 0)
        // check secret pin for admin sign up
        {
            while (1)
            {
                strcpy(secret_pin, getpass("\tEnter secret PIN to create ADMIN account: "));
                if (strcmp(secret_pin, "secret") != 0)
                    printf("\tInvalid PIN. Please Try Again.\n");
                else
                    break;
            }
        }

        write(sock, &type, sizeof(type));
        write(sock, &name, sizeof(name));
        write(sock, &password, strlen(password));

        read(sock, &id, sizeof(id));
        printf("\tRemember Your login id For Further Logins as :: %d\n", id);
        return 2;
    }
    else // Log out
        return 3;
}

//-------------------- Main menu function-----------------------------//

int menu(int sock, int type)
{
    int choice;
    if (type == 2 || type == 1)
    { // Agent or Customer
        printf("\t1. Book Ticket\n");
        printf("\t2. View Bookings\n");
        printf("\t3. Update Booking\n");
        printf("\t4. Cancel booking\n");
        printf("\t5. Logout\n");
        printf("\tYour Choice: ");
        scanf("%d", &choice);
        write(sock, &choice, sizeof(choice));
        printf("You chose choice: %d\n", choice);
        return user_function(sock, choice);
    }
    else if (type == 0)
    { // Admin
        printf("\n\t1. CRUD operations on train\n");
        printf("\t2. CRUD operations on user\n");
        printf("\t3. Logout\n");
        printf("\t Your Choice: ");
        scanf("%d", &choice);
        write(sock, &choice, sizeof(choice));
        if (choice == 1)
        {
            printf("\t1. Add train\n");
            printf("\t2. View train\n");
            printf("\t3. Modify train\n");
            printf("\t4. Delete train\n");
            printf("\t Your Choice: ");
            scanf("%d", &choice);
            write(sock, &choice, sizeof(choice));
            return crud_train(sock, choice);
        }
        else if (choice == 2)
        {
            printf("\t1. Add User\n");
            printf("\t2. View all users\n");
            printf("\t3. Modify user\n");
            printf("\t4. Delete user\n");
            printf("\t Your Choice: ");
            scanf("%d", &choice);
            write(sock, &choice, sizeof(choice));
            return crud_user(sock, choice);
        }
        else if (choice == 3)
            return -1;
    }
}

//-------------------------------- User function to book tickets -----------------------------//
int user_function(int sock, int choice)
{
    int valid = 0;
    if (choice == 1)
    { // Book tickets
        int view = 2, tid, seats;
        write(sock, &view, sizeof(int));
        crud_train(sock, view);
        printf("\n\tEnter the train number you want to book: ");
        scanf("%d", &tid);
        write(sock, &tid, sizeof(tid));

        printf("\n\tEnter the no. of seats you want to book: ");
        scanf("%d", &seats);
        write(sock, &seats, sizeof(seats));

        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\tTicket booked successfully.\n");
        else
            printf("\n\tSeats were not available.\n");

        return valid;
    }

    else if (choice == 2)
    { // View the bookings
        int no_of_bookings;
        int id, tid, seats;
        read(sock, &no_of_bookings, sizeof(no_of_bookings));

        printf("\tB_id\tT_no\tSeats\n");
        while (no_of_bookings--)
        {
            read(sock, &id, sizeof(id));
            read(sock, &tid, sizeof(tid));
            read(sock, &seats, sizeof(seats));

            if (seats != 0)
                printf("\t%d\t%d\t%d\n", id, tid, seats);
        }

        return valid;
    }

    else if (choice == 3)
    { // Update a booking (increment/ decrement seats)
        int choice = 2, bid, val, valid;
        user_function(sock, choice);
        printf("\n\t Enter the B_id you want to modify: ");
        scanf("%d", &bid);
        write(sock, &bid, sizeof(bid));

        printf("\n\t1. Increase number of seats\n\t2. Decrease number of seats\n");
        printf("\t Your Choice: ");
        scanf("%d", &choice);
        write(sock, &choice, sizeof(choice));

        if (choice == 1)
        {
            printf("\n\tNo. of tickets to increase");
            scanf("%d", &val);
            write(sock, &val, sizeof(val));
        }
        else if (choice == 2)
        {
            printf("\n\tNo. of tickets to decrease");
            scanf("%d", &val);
            write(sock, &val, sizeof(val));
        }
        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\tBooking updated successfully.\n");
        else
            printf("\n\tUpdation failed. No more seats available.\n");
        return valid;
    }

    else if (choice == 4)
    { // Cancel the entire booking
        int choice = 2, bid, valid;
        user_function(sock, choice);
        printf("\n\t Enter the B_id you want to cancel: ");
        scanf("%d", &bid);
        write(sock, &bid, sizeof(bid));
        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\tBooking cancelled successfully.\n");
        else
            printf("\n\tCancellation failed.\n");
        return valid;
    }
    else if (choice == 5) // Logout
        return -1;
}