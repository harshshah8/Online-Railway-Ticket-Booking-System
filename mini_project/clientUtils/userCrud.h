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

//-------------------------------- crud operations on user-----------------------------//

int crud_user(int sock, int choice)
{
    int valid = 0;
    if (choice == 1)
    { // Add user
        int type, id;
        char name[50], password[50];
        printf("\n\tEnter The Type Of Account:: \n");
        printf("\t1. Agent\n\t2. Customer\n");
        printf("\tYour Response: ");
        scanf("%d", &type);
        printf("\tUser Name: ");
        scanf("%s", name);
        strcpy(password, getpass("\tPassword: "));
        write(sock, &type, sizeof(type));
        write(sock, &name, sizeof(name));
        write(sock, &password, strlen(password));
        read(sock, &valid, sizeof(valid));
        if (valid)
        {
            read(sock, &id, sizeof(id));
            printf("\tRemember Your login id For Further Logins as :: %d\n", id);
        }
        return valid;
    }

    else if (choice == 2)
    { // View user list
        int no_of_users;
        int id, type;
        char uname[50];
        read(sock, &no_of_users, sizeof(no_of_users));

        printf("\tU_id\tU_name\tU_type\n");
        while (no_of_users--)
        {
            read(sock, &id, sizeof(id));
            read(sock, &uname, sizeof(uname));
            read(sock, &type, sizeof(type));

            if (strcmp(uname, "deleted") != 0)
                printf("\t%d\t%s\t%d\n", id, uname, type);
        }

        return valid;
    }

    else if (choice == 3)
    { // Update user
        int choice = 2, valid = 0, uid;
        char name[50], pass[50];
        write(sock, &choice, sizeof(int));
        crud_user(sock, choice);
        printf("\n\t Enter the U_id you want to modify: ");
        scanf("%d", &uid);
        write(sock, &uid, sizeof(uid));

        printf("\n\t1. User Name\n\t2. Password\n");
        printf("\t Your Choice: ");
        scanf("%d", &choice);
        write(sock, &choice, sizeof(choice));

        if (choice == 1)
        {
            read(sock, &name, sizeof(name));
            printf("\n\t Current name: %s", name);
            printf("\n\t Updated name:");
            scanf("%s", name);
            write(sock, &name, sizeof(name));
            read(sock, &valid, sizeof(valid));
        }
        else if (choice == 2)
        {
            printf("\n\t Enter Current password: ");
            scanf("%s", pass);
            write(sock, &pass, sizeof(pass));
            read(sock, &valid, sizeof(valid));
            if (valid)
            {
                printf("\n\t Enter new password:");
                scanf("%s", pass);
            }
            else
                printf("\n\tIncorrect password\n");

            write(sock, &pass, sizeof(pass));
        }
        if (valid)
        {
            read(sock, &valid, sizeof(valid));
            if (valid)
                printf("\n\t User data updated successfully\n");
        }
        return valid;
    }

    else if (choice == 4)
    { // Delete a user
        int choice = 2, uid, valid = 0;
        write(sock, &choice, sizeof(int));
        crud_user(sock, choice);

        printf("\n\t Enter the id you want to delete: ");
        scanf("%d", &uid);
        write(sock, &uid, sizeof(uid));
        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\t User deleted successfully\n");
        return valid;
    }
}
