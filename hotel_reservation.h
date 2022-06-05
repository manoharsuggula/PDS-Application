#ifndef HOTEL_RESRVATION_H
#define HOTEL_RESRVATION_H

#define RESERVATION_SUCCESS 0
#define RESERVATION_FAILURE 1
 
struct Reservation{
	int reservation_id;
	int room_no;
	char customer_name[30];
	char phone[15];
};

extern struct PDS_RepoInfo *repoHandle;

// Add the given Reservation into the repository by calling put_rec_by_key
int add_reservation( struct Reservation *r );

// Display Reservation info in a single line as a CSV without any spaces
void print_reservation( struct Reservation *r );

// Use get_rec_by_key function to retrieve Reservation
int search_reservation( int Reservation_id, struct Reservation *r );

int print_all_reservation( char *Reservation_data_file );

// Use get_rec_by_non_ndx_key function to retrieve Reservation
int search_reservation_by_phone( char *phone, struct Reservation *r, int *io_count );

/* Return 0 if phone of the Reservation matches with phone parameter */
/* Return 1 if phone of the Reservation does NOT match */
/* Return > 1 in case of any other error */
int match_reservation_phone( void *rec, void *key );

// Function to delete Reservation by ID
int delete_reservation ( int Reservation_id );

int overwrite_reservation( struct Reservation *r );

#endif
