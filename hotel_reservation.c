#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "hotel_reservation.h"

// Add the given Reservation into the repository by calling put_rec_by_key
int add_reservation( struct Reservation *r )
{
	int status;

	status = put_rec_by_key( r->reservation_id, r );

	if( status == PDS_ADD_FAILED)
	{
		fprintf(stderr, "Unable to add reservation with key %d. ", r->reservation_id);
		return status;
	}
	return status;
}

// Display Reservation info in a single line as a CSV without any spaces
void print_reservation( struct Reservation *r )
{
	printf("********************************\n");
	printf("  Reservation id : %d\n  Customer Name : %s\n  Phone Number : %s\n  Room Number : %d\n", r->reservation_id,r->customer_name,r->phone,r->room_no);
	printf("********************************\n");
}

// Use get_rec_by_key function to retrieve Reservation
int search_reservation( int reservation_id, struct Reservation *r )
{
	return get_rec_by_ndx_key( reservation_id, r );
}

// Load all the Reservations from a CSV file
int print_all_reservation( char *reservation_data_file )
{
	FILE *dat_fptr, *ndx_fptr;
	struct Reservation r, dummy;
	int k;

	char dat_file[20], ndx_file[20];
	strcpy(dat_file,reservation_data_file);
	strcpy(ndx_file,reservation_data_file);
	strcat(dat_file,".dat");
	strcat(ndx_file,".ndx");
	dat_fptr = fopen(dat_file, "rb");
	ndx_fptr = fopen(ndx_file, "rb");
	fseek(ndx_fptr, 0, SEEK_END);
    int last_offset = ftell(ndx_fptr);
    fseek(ndx_fptr,0,SEEK_SET);
	int count = 0;
	while(ftell(ndx_fptr) != last_offset)
	{
		struct PDS_NdxInfo *ndxinfo = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
        int status = fread(ndxinfo, sizeof(struct PDS_NdxInfo), 1, ndx_fptr);
        if(status != 1)
		{
			printf("Error\n");
            return PDS_LOAD_NDX_FAILED;
        }
		fseek(dat_fptr,ndxinfo->offset,SEEK_SET);
		fread(&k, sizeof(int), 1, dat_fptr);
		fread(&r, sizeof(struct Reservation), 1, dat_fptr);
		printf("\n");
		print_reservation( &r );
		count++;
	}

	if(count == 0)
	{
		printf("\nThere are no reservations right now.");
	}

	fclose(dat_fptr);
	fclose(ndx_fptr);

}

// Use get_rec_by_non_ndx_key function to retrieve Reservation
int search_reservation_by_phone( char *phone, struct Reservation *r, int *io_count )
{
	return get_rec_by_non_ndx_key( phone, r, &match_reservation_phone, io_count );
}

/* Return 0 if phone of the Reservation matches with phone parameter */
/* Return 1 if phone of the Reservation does NOT match */
/* Return > 1 in case of any other error */
int match_reservation_phone( void *rec, void *key )
{
	struct Reservation *r = rec;
    // Store the key in a char pointer
	char* k = key;
    // Compare the phone values in key and record
	int comp = strcmp(r->phone,k);
    // Return 0,1,>1 based on above condition
	if(!comp)
	{
		return 0;
	}
	else if(comp != 0)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

// Function to delete Reservation by ID
int delete_reservation ( int reservation_id )
{
	int status = delete_rec_by_ndx_key(reservation_id);
	if(status == PDS_SUCCESS)
	{
		return RESERVATION_SUCCESS;
	}
	else
	{
		return RESERVATION_FAILURE;
	}
}

int overwrite_reservation( struct Reservation *r )
{
	int status = pds_overwrite(r->reservation_id, r);
	// return status;
	if(status == PDS_SUCCESS)
	{
		return RESERVATION_SUCCESS;
	}
	else
	{
		return RESERVATION_FAILURE;
	}
}

