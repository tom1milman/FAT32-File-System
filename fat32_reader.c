/***********************************************************
 * TODO: Fill in this area and delete this line
 * Name of program:
 * Authors:
 * Description:
 **********************************************************/

/* These are the included libraries.  You may need to add more. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include "bpb_info.h"

/* Put any symbolic constants (defines) here */
#define True 1  /* C has no booleans! */
#define False 0

#define MAX_CMD 80

int parse_args (int argc, char **argv, char *file_path);
unsigned char * read_file (char *file_path);
int check_510_511_bytes (unsigned char * buffer);
unsigned int getFirstDataSector (BPB_Info bpb_info);
unsigned int getFirstSectorofClusterN (int N, BPB_Info bpb_info, unsigned int FirstDataSector);


/* This is the main function of your project, and it will be run
 * first before all other functions.
 */
int main(int argc, char *argv[])
{
    char cmd_line[MAX_CMD];
    char disk_image_path[MAX_CMD];


    /* Parse args and open our image file */
    parse_args(argc, argv, disk_image_path);

    unsigned char * buffer;
    buffer = read_file(disk_image_path);

    if (check_510_511_bytes(buffer))
        return 1;


    /* Parse boot sector and get information */
    BPB_Info bpb_info = get_BPB_info(buffer);

    unsigned int FirstDataSector = getFirstDataSector(bpb_info);

    if (FirstDataSector == -1) {
        printf("Disk image is not a FAT32 file system. ");
        return 1;
    }

    /* Get root directory address */
    unsigned int FirstSectorofCluster2 = getFirstSectorofClusterN(2, bpb_info, FirstDataSector);
    unsigned int root_addr = FirstSectorofCluster2 * bpb_info.BPB_BytesPerSec;

    printf("Root addr is 0x%x\n", root_addr);

//    for (int i = root_addr; i < root_addr + 20; i++) {
//        printf("%.2X ", buffer[i]);
//    }

    printf("\n");

    unsigned int FATSz;

    if (bpb_info.BPB_FATSz32 != 0)
        FATSz = bpb_info.BPB_FATSz32;
    //if ()


    unsigned int current_directory = root_addr;

    /* Main loop.  You probably want to create a helper function
       for each command besides quit. */

    while(True) {
        bzero(cmd_line, MAX_CMD);
        printf("/]");
        fgets(cmd_line,MAX_CMD,stdin);

        /* Start comparing input */
        if(strncmp(cmd_line,"info",4)==0) {
            print_BPB_info(&bpb_info);
            break;
        }

        else if(strncmp(cmd_line,"open",4)==0) {
            printf("Going to open!\n");
        }

        else if(strncmp(cmd_line,"close",5)==0) {
            printf("Going to close!\n");
        }

        else if(strncmp(cmd_line,"size",4)==0) {
            printf("Going to size!\n");
        }

        else if(strncmp(cmd_line,"cd",2)==0) {
            printf("Going to cd!\n");
        }

        else if(strncmp(cmd_line,"ls",2)==0) {
            printf("Going to ls.\n");
        }

        else if(strncmp(cmd_line,"read",4)==0) {
            printf("Going to read!\n");
        }

        else if(strncmp(cmd_line,"quit",4)==0) {
            printf("Quitting.\n");
            break;
        }
        else
            printf("Unrecognized command.\n");

    }

    /* Close the file */
    printf("Closing file...\n");
    free(buffer);

    return 0; /* Success */
}

int parse_args (int argc, char **argv, char *file_path)
{
    int i=0;

    for(i=0;i<argc;i++) {
        if (i == 1) {
            printf("file path %s\n", argv[i]);
            strncpy(file_path, argv[i], MAX_CMD);
        }
    }

    return 0;
}

unsigned char * read_file (char *file_path)
{
    FILE *fileptr;
    unsigned char *buffer;
    long filelen;

    fileptr = fopen(file_path, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    buffer = (char *)malloc((filelen+1)*sizeof(char)); // Enough memory for file + \0
    fread(buffer, filelen, 1, fileptr); // Read in the entire file

    fclose(fileptr); // Close the file

    return buffer;
}

int check_510_511_bytes (unsigned char * buffer)
{
    /*Check if the 510-th byte is equal to 0x55*/
    if ((int) buffer[510] != 85) {
        printf("Byte 510: %.2X != 0x55 \n", (int) buffer[510]);
        return 1;
    }

    /*Check if the 511-th byte is equal to 0xAA*/
    if ((int) buffer[511] != 170) {
        printf("Byte 511: %.2X != 0xAA \n", (int) buffer[511]);
        return 1;
    }

    return 0;
}

unsigned int getFirstDataSector (BPB_Info bpb_info) {
    unsigned int FATSz;

    if (bpb_info.BPB_FATSz32 != 0)
        FATSz = bpb_info.BPB_FATSz32;
    else
        return -1;

    unsigned int FirstDataSector = bpb_info.BPB_RsvdSecCnt + (bpb_info.BPB_NumFATs * FATSz) + 0;

    return FirstDataSector;
}

unsigned int getFirstSectorofClusterN (int N, BPB_Info bpb_info, unsigned int FirstDataSector)
{
    unsigned int FirstSectorofCluster = ((N - 2) * bpb_info.BPB_SecPerClus) + FirstDataSector;

    return FirstDataSector;
}

int cd_function ()
{
    return 0;
}

int ls_function ()
{
    return 0;
}