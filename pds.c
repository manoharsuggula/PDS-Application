#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

#include "pds.h"
#include "bst.h"

struct PDS_RepoInfo repo_handle;


int pds_create(char *repo_name) 
{
	char filename[30], indexfile[30];
	strcpy(filename,repo_name);
	strcpy(indexfile,repo_name);
	strcat(filename,".dat");
	strcat(indexfile,".ndx");
	FILE *fp = fopen(filename,"wb+");
	FILE *ifp = fopen(indexfile,"wb+");
	if(fp  == NULL || ifp == NULL) return PDS_FILE_ERROR;
	fclose(fp);
	fclose(ifp);
	
	return PDS_SUCCESS;
}


int pds_open(char* repo_name, int rec_size) // Same as before
{
	if(repo_handle.repo_status == PDS_REPO_OPEN)
	{
		return PDS_REPO_ALREADY_OPEN;
	}

	char ndx_file[30],dat_file[30];
	strcpy(ndx_file,repo_name);
	strcpy(dat_file,repo_name);
	strcat(dat_file,".dat");
	strcat(ndx_file,".ndx");
// Open the data file and index file in rb+ mode
	FILE* file_ptr_dat = fopen(dat_file,"rb+");
	FILE* file_ptr_ndx = fopen(ndx_file,"rb+");
	if(file_ptr_dat == NULL)
	{
		return PDS_FILE_ERROR;
	}
	if(file_ptr_ndx == NULL)
	{
		return PDS_FILE_ERROR;
	}
// Update the fields of repo_handle appropriately
	strcpy(repo_handle.pds_name,repo_name);
	repo_handle.pds_data_fp = file_ptr_dat;
	repo_handle.pds_ndx_fp = file_ptr_ndx;
	repo_handle.repo_status = PDS_REPO_OPEN;
	repo_handle.rec_size = rec_size;
	// struct BST_Node *root = NULL;
	repo_handle.pds_bst = NULL;
// Build BST and store in pds_bst by reading index entries from the index file
	int status = pds_load_ndx();
	if(status != PDS_SUCCESS)
	{
		return PDS_LOAD_NDX_FAILED;
	}
// Close only the index file
	fclose(repo_handle.pds_ndx_fp);
	return PDS_SUCCESS;

}

int pds_load_ndx() // Same as before
{
	fseek(repo_handle.pds_ndx_fp, 0, SEEK_END);
    int last_offset = ftell(repo_handle.pds_ndx_fp);
    fseek(repo_handle.pds_ndx_fp,0,SEEK_SET);
	while(ftell(repo_handle.pds_ndx_fp) != last_offset)
	{
		struct PDS_NdxInfo *ndxinfo = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
        int status = fread(ndxinfo, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp);
        
		if(status != 1){
            return PDS_LOAD_NDX_FAILED;
        }

        status = bst_add_node(&repo_handle.pds_bst, ndxinfo->key, ndxinfo);
        if(status != BST_SUCCESS){
            return PDS_LOAD_NDX_FAILED;
        }

	}
	return PDS_SUCCESS;
}

int put_rec_by_key(int key, void*rec)
{
 	if(repo_handle.repo_status == PDS_REPO_OPEN)
    {
		// Seek to the end of the data file
		fseek(repo_handle.pds_data_fp, 0, SEEK_END);
		int offset = ftell(repo_handle.pds_data_fp);
// Create an index entry with the current data file location using ftell
		struct PDS_NdxInfo* ndxinfo = (struct PDS_NdxInfo*)malloc(sizeof(struct PDS_NdxInfo));
		ndxinfo->key = key;
		ndxinfo->offset = offset;
		ndxinfo->is_deleted = 0;
// Add index entry to BST using offset returned by ftell
		int status = bst_add_node(&repo_handle.pds_bst,ndxinfo->key,ndxinfo);
        if(status != BST_SUCCESS)
		{
			return PDS_ADD_FAILED;
		}
// Write the key at the current data file location
// Write the record after writing the key
		if(fwrite(&key, sizeof(int), 1, repo_handle.pds_data_fp) == 1 && fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp) == 1)
        {
            return PDS_SUCCESS;
        }
        else
        {
            return PDS_ADD_FAILED;
        }
    }
    else
    {
        return PDS_REPO_NOT_OPEN;
    }
}

int get_rec_by_ndx_key(int key, void*rec)
{
	if(repo_handle.repo_status == PDS_REPO_OPEN)
    {
// Search for index entry in BST
		struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
		if(node == NULL)
		{
			return PDS_REC_NOT_FOUND;
		}
		int k,offset;
		struct PDS_NdxInfo* data;
		data = node->data;
		if(data->is_deleted == 1)
		{
			return PDS_REC_NOT_FOUND;
		}
		offset = data->offset;
// Seek to the file location based on offset in index entry
		fseek(repo_handle.pds_data_fp,offset,SEEK_SET);
// Read the key at the current file location 
		fread(&k, sizeof(int), 1, repo_handle.pds_data_fp);
// Read the record after reading the key
		fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
		if(key == k)
		{
			return PDS_SUCCESS;
		}
		else
		{
			return PDS_REC_NOT_FOUND;
		}
	}
	return PDS_REPO_NOT_OPEN;
}



int pds_close() 
{
	int pre_order(struct BST_Node *root)
	{
		if( root == NULL)
		{
			return PDS_SUCCESS;
		}
		struct PDS_NdxInfo *ndxinfo = (struct PDS_NdxInfo*)root->data;
		if(ndxinfo->is_deleted != 1)
		{
			int status = fwrite(ndxinfo, sizeof(struct PDS_NdxInfo), 1, repo_handle.pds_ndx_fp);
			if(status != 1)
			{
				return PDS_NDX_SAVE_FAILED;
			}
		}
		int s1 = pre_order(root->left_child);
		int s2 = pre_order(root->right_child);
		if(s1 == PDS_SUCCESS && s2 == PDS_SUCCESS)
		{
			return PDS_SUCCESS;
		}
		else
		{
			return PDS_NDX_SAVE_FAILED;
		}
	}

	if(repo_handle.repo_status == PDS_REPO_OPEN)
    {
		char ndx_file[30];
		strcpy(ndx_file,repo_handle.pds_name);
		strcat(ndx_file,".ndx");
// Open the index file in wb mode (write mode, not append mode)
		FILE* file_ptr_ndx = fopen(ndx_file,"wb");
		if(file_ptr_ndx == NULL)
		{
			return PDS_FILE_ERROR;
		}
		repo_handle.pds_ndx_fp = file_ptr_ndx;
// Unload the BST into the index file by traversing it in PRE-ORDER (overwrite the entire index file)
		int status = pre_order(repo_handle.pds_bst);
		if(status != PDS_SUCCESS)
		{
			return PDS_NDX_SAVE_FAILED;
		}
// Free the BST by calling bst_destroy()
		bst_destroy(repo_handle.pds_bst);
// Close the index file and data file
		fclose(repo_handle.pds_data_fp);
		fclose(repo_handle.pds_ndx_fp);
		strcpy(repo_handle.pds_name, "");
		repo_handle.pds_data_fp = NULL;
		repo_handle.pds_ndx_fp = NULL;
		repo_handle.repo_status = PDS_REPO_CLOSED;
		repo_handle.rec_size = 0;
		return PDS_SUCCESS;
	}
	return PDS_REPO_CLOSED;
}

int get_rec_by_non_ndx_key(void *key, void *rec, int (*matcher)(void *rec, void *key), int *io_count)
{
	if(repo_handle.repo_status == PDS_REPO_OPEN)
    {
  		// Seek to beginning of file
		fseek(repo_handle.pds_data_fp,0,SEEK_SET);
		int k;
		// Perform a table scan - iterate over all the records
		while(!feof(repo_handle.pds_data_fp))
		{
		  	//Read the key and the record
			fread(&k, sizeof(int), 1, repo_handle.pds_data_fp);
			fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
  //   Increment io_count by 1 to reflect count no. of records read
			(*io_count)++;
  //   Use the function in function pointer to compare the record with required key
			int output = matcher(rec,key);
			if(output == 0)
			{
				struct BST_Node *node = bst_search(repo_handle.pds_bst, k);
				if(node == NULL)
				{
					return PDS_REC_NOT_FOUND;
				}
				struct PDS_NdxInfo* data;
				data = node->data;
				if(data->is_deleted == 1)
				{
					return PDS_REC_NOT_FOUND;
				}
  // Return success when record is found
				return PDS_SUCCESS;
			}
		}
		return PDS_REC_NOT_FOUND;
	}
	return PDS_REPO_NOT_OPEN;
}

int delete_rec_by_ndx_key( int key) // New Function
{
	struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
	if(node == NULL)
	{
		return PDS_DELETE_FAILED;
	}
	struct PDS_NdxInfo* data;
	data = node->data;
	if(data->is_deleted == 1)
	{
		return PDS_DELETE_FAILED;
	}
	data->is_deleted = 1;
	return PDS_SUCCESS;
}

// Overwrite an existing record with a new record
// Return relevant error code if record does not exist or any other error
// This implementation overwrites existing record with the given object
int pds_overwrite( int key, void *rec )
{
	if(repo_handle.repo_status == PDS_REPO_OPEN)
    {
		struct BST_Node *node = bst_search(repo_handle.pds_bst, key);
		if(node == NULL)
		{
			return PDS_UPDATE_FAILED;
		}
		int k,offset;
		struct PDS_NdxInfo* data;
		data = node->data;
		if(data->is_deleted == 1)
		{
			return PDS_UPDATE_FAILED;
		}
		offset = data->offset;
		// Seek to the file location based on offset in index entry
		fseek(repo_handle.pds_data_fp,offset,SEEK_SET);
		// Read the key at the current file location 
		fread(&k, sizeof(int), 1, repo_handle.pds_data_fp);
		// Read the record after reading the key
		int write = fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
		if(write == 1)
		{
			return PDS_SUCCESS;
		}
		else
		{
			return PDS_UPDATE_FAILED;
		}
	}
	return PDS_REPO_NOT_OPEN;	
}

