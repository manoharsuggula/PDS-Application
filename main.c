#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "hotel_reservation.h"

#define TREPORT(a1,a2) printf("Status %s: %s",a1,a2); fflush(stdout);


void dashboard()
{
    printf("Choose an option to perform:\n");
    printf("\t1 : Add a new reservation\n");
    printf("\t2 : Search a Reservation by id\n");
    printf("\t3 : Search a Reservation by phone number\n");
    printf("\t4 : Delete a Reservation by id\n");
    printf("\t5 : Update the details of Reservation\n");
    printf("\t6 : List all the reservations\n");
    printf("\t9 : Refresh\n");
    printf("\t0 : Exit\n\n");
    printf("Enter a number to perform corresponding action : ");
}

int main()
{
    int running = 1,status;
    char repo_name[20], info[1000];
    struct Reservation r;
    int rec_size = sizeof(r);
    strcpy(repo_name,"newdemo");
    pds_create(repo_name);
    pds_open(repo_name, rec_size);
    int id,add_count=0,count=0;
    char inp[2];
    char phone_no[15];
    while(running)
    {
        dashboard();
        int input;
        scanf("%d",&input);
        switch(input)
        {
            case 1:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to add a Reservation\n");
                printf("Total records added till now : %d\n",add_count);
                printf("Total records at present : %d\n\n",count);
                printf("Please Enter the id : ");
                scanf("%d",&r.reservation_id);
                printf("Please Enter the Name : ");
                scanf("%s",r.customer_name);
                printf("Please Enter the Phone Number : ");
                scanf("%s",r.phone);
                printf("Please Enter the room number : ");
                scanf("%d",&r.room_no);
                status = add_reservation(&r);
                if(status == PDS_SUCCESS)
                {
                    count++;
                    add_count++;
                    sprintf(info,"Reservation record added succesfully.");
                    TREPORT("SUCCESS",info);
                }
                else 
                {
                    sprintf(info,"Reservation record add failed.");
                    TREPORT("FAILED",info);
                }
                printf("\n---------------------------------------------------------------------------------\n\n");
                break;

            case 2:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to search a Reservation by its id\n");
                printf("Please Enter the reservation id : ");
                scanf("%d",&id);
                status = search_reservation(id,&r);
                if(status == RESERVATION_SUCCESS)
                {
                    sprintf(info,"Reservation record is");
                    TREPORT("SUCCESS",info);
                    printf("\n\n");
                    print_reservation(&r);
                }
                else
                {
                    sprintf(info,"Reservation Not Found!! Enter a valid id");
                    TREPORT("FAILED",info);
                }
                printf("\n---------------------------------------------------------------------------------\n\n");
                break;

            case 3:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to search a Reservation by its phone number\n");
                printf("Please Enter the phone number : ");
                scanf("%s",phone_no);
                int actual_io=0;
                status = search_reservation_by_phone(phone_no, &r, &actual_io);
                if(status == RESERVATION_SUCCESS)
                {
                    sprintf(info,"Reservation record is");
                    TREPORT("SUCCESS",info);
                    printf("\n\n");
                    print_reservation(&r);
                }
                else
                {
                    sprintf(info,"Reservation Not Found!! Enter the correct phone number");
                    TREPORT("FAILED",info);
                }
                printf("\n---------------------------------------------------------------------------------\n\n");
                break;

            case 4:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to Delete a Reservation by its id\n");
                printf("Please Enter the reservation id : ");
                scanf("%d",&id);
                printf("If you delete, you can't access the record. Are you sure you want to delete?[y/n] : ");
                scanf("%s",inp);
                if(inp[0] == 'y' || inp[0] == 'Y')
                {
                    status = delete_reservation(id);
                    if(status == RESERVATION_SUCCESS)
                    {
                        count--;
                        pds_close();
                        pds_open(repo_name, rec_size);
                        sprintf(info,"Reservation record deleted succesfully.");
                        TREPORT("SUCCESS",info);
                    }
                    else
                    {
                        sprintf(info,"Reservation record delete failed.");
                        TREPORT("FAILED",info);
                    }
                }
                printf("\n---------------------------------------------------------------------------------\n\n");
                break;
            
            case 5:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to Update a Reservation\n");
                printf("Please Enter the reservation id to overwrite : ");
                scanf("%d",&id);
                printf("Please Enter the Name : ");
                scanf("%s",r.customer_name);
                printf("Please Enter the Phone Number : ");
                scanf("%s",r.phone);
                printf("Please Enter the room number : ");
                scanf("%d",&r.room_no);
                r.reservation_id = id;
                status = overwrite_reservation(&r);
                if(status == RESERVATION_SUCCESS)
                {
                    sprintf(info,"Reservation record updated succesfully.");
                    TREPORT("SUCCESS",info);
                }
                else
                {
                    sprintf(info,"Reservation record update failed.");
                    TREPORT("FAILED",info);
                }
                printf("\n---------------------------------------------------------------------------------\n\n");
                break;

            case 6:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to print all existing reservations\n");
                pds_close();
                print_all_reservation(repo_name);
                pds_open(repo_name, rec_size);
                printf("\n");
                printf("\n---------------------------------------------------------------------------------\n\n");
                break;

            case 9:
                printf("\n---------------------------------------------------------------------------------\n");
                printf("You have selected to Refresh\n\n");
                status = pds_close();
                status = pds_open(repo_name, rec_size);

                printf("\n---------------------------------------------------------------------------------\n\n");
                system("sleep 0.5");
                break;               

            case 0:
                printf("\nYou have selected to exit.\n");
                printf("If you exit, the program will end. Are you sure you want to exit[y/n] : ");
                scanf("%s",inp);
                if(inp[0] == 'y' || inp[0] == 'Y')
                {
                    pds_close();
                    printf("Thank you!!\n");
                    running = 0;
                    exit(0);
                }
                printf("\n\n");
                break;
        }
    }

}