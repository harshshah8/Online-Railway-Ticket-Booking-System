/*
Author: Harsh Shah
Roll: MT2021050
*/

#include <stdio.h>

//--------------- structures declaration of train and user------------//
struct train
{
    int train_number;
    char train_name[50];
    int total_seats;
    int available_seats;
};
struct user
{
    int login_id;
    char password[50];
    char name[50];
    int type;
};

struct booking
{
    int booking_id;
    int type;
    int uid;
    int tid;
    int seats;
};
//---------------------------------------------------------------------//
