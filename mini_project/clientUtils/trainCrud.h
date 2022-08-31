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

//-------------------------------- crud operations on train-----------------------------//
int crud_train(int sock, int choice)
{
    int valid = 0;
    if (choice == 1)
    { // Add train response
        char tname[50];
        printf("\n\tEnter train name: ");
        scanf("%s", tname);
        write(sock, &tname, sizeof(tname));
        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\tTrain added successfully\n");

        return valid;
    }

    else if (choice == 2)
    { // View train response
        int no_of_trains;
        int tno;
        char tname[50];
        int tseats;
        int aseats;
        read(sock, &no_of_trains, sizeof(no_of_trains));

        printf("\tT_no\tT_name\tT_seats\tA_seats\n");
        while (no_of_trains--)
        {
            read(sock, &tno, sizeof(tno));
            read(sock, &tname, sizeof(tname));
            read(sock, &tseats, sizeof(tseats));
            read(sock, &aseats, sizeof(aseats));

            if (strcmp(tname, "deleted") != 0)
                printf("\t%d\t%s\t%d\t%d\n", tno, tname, tseats, aseats);
        }

        return valid;
    }

    else if (choice == 3)
    { // Update train response
        int tseats, choice = 2, valid = 0, tid;
        char tname[50];
        write(sock, &choice, sizeof(int));
        crud_train(sock, choice);
        printf("\n\t Enter the train number you want to modify: ");
        scanf("%d", &tid);
        write(sock, &tid, sizeof(tid));

        printf("\n\t1. Train Name\n\t2. Total Seats\n");
        printf("\t Your Choice: ");
        scanf("%d", &choice);
        write(sock, &choice, sizeof(choice));

        if (choice == 1)
        {
            read(sock, &tname, sizeof(tname));
            printf("\n\t Current name: %s", tname);
            printf("\n\t Updated name:");
            scanf("%s", tname);
            write(sock, &tname, sizeof(tname));
        }
        else if (choice == 2)
        {
            read(sock, &tseats, sizeof(tseats));
            printf("\n\t Current value: %d", tseats);
            printf("\n\t Updated value:");
            scanf("%d", &tseats);
            write(sock, &tseats, sizeof(tseats));
        }
        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\t Train data updated successfully\n");
        return valid;
    }

    else if (choice == 4)
    { // Delete train response
        int choice = 2, tid, valid = 0;
        write(sock, &choice, sizeof(int));
        crud_train(sock, choice);

        printf("\n\t Enter the train number you want to delete: ");
        scanf("%d", &tid);
        write(sock, &tid, sizeof(tid));
        read(sock, &valid, sizeof(valid));
        if (valid)
            printf("\n\t Train deleted successfully\n");
        return valid;
    }
}
