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
#include "dir_object.h"
#include "token_object.h"

/* Put any symbolic constants (defines) here */
#define True 1  /* C has no booleans! */
#define False 0

#define MAX_CMD 80

int parse_args (int argc, char **argv, char *file_path);
unsigned char * read_disk (char *file_path);
int check_510_511_bytes (unsigned char * buffer);
unsigned int getFirstDataSector (BPB_Info bpb_info);
//unsigned int getFirstSectorofClusterN (unsigned int clusterNum, BPB_Info bpb_info, unsigned int FirstDataSector);
//unsigned int * getAllClusters (unsigned int firstClus, unsigned char * buffer, unsigned int FAToffset);
int ls_function (DIR_files files);
int read_file (int start, int end, unsigned int address, char * buffer);
int stats_function (DIR_info * file);
char * change_terminalStr (char * terminalStrOld, char * new_dir);

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
    buffer = read_disk(disk_image_path);

    if (check_510_511_bytes(buffer))
        return 1;


    /* Parse boot sector and get information */
    BPB_Info bpb_info = get_BPB_info(buffer);

    unsigned int FirstDataSector = getFirstDataSector(bpb_info);

    if (FirstDataSector == -1) {
        printf("Disk image is not a FAT32 file system. ");
        return 1;
    }

    unsigned int FAToffset = bpb_info.BPB_BytesPerSec * bpb_info.BPB_RsvdSecCnt;

    /* Get root directory address */
    unsigned int FirstSectorofCluster2 = getFirstSectorofClusterN(bpb_info.BPB_RootClus, bpb_info.BPB_SecPerClus, FirstDataSector);
    unsigned int root_addr = FirstSectorofCluster2 * bpb_info.BPB_BytesPerSec;

    unsigned int FATSz;

    if (bpb_info.BPB_FATSz32 != 0)
        FATSz = bpb_info.BPB_FATSz32;

    unsigned int current_directory = root_addr;
    DIR_info current_dir_info;
    DIR_files files;

    char * terminalStr = "/]";

    /* Main loop.  You probably want to create a helper function
       for each command besides quit. */

    current_dir_info = dir_constructor(buffer, current_directory, 0);
    files = get_dir_files(buffer, current_dir_info, bpb_info.BPB_SecPerClus, bpb_info.BPB_BytesPerSec, FirstDataSector, FAToffset);

    while(True) {
        bzero(cmd_line, MAX_CMD);
        printf("%s", terminalStr);
        fgets(cmd_line,MAX_CMD,stdin);

        token_obj cmd_tokens = getTokens(cmd_line);

        /* Start comparing input */
        if(strncmp(cmd_line,"info",4)==0) {
            print_BPB_info(&bpb_info);
        }

        else if(strncmp(cmd_line,"size",4)==0) {
            if (cmd_tokens.num_of_tokens < 2)
                printf("Error: not enough args\n");

            DIR_info * size_file = NULL;
            size_file = get_file_from(files, cmd_tokens.tokens[1]);

            if (size_file == NULL) {
                printf("Error: does not exist\n");
                continue;
            }

            printf("Size: %d\n", size_file->DIR_FileSize);
        }

        else if(strncmp(cmd_line,"volume",6)==0) {
            printf("Volume: %s\n", volume_name);
        }

        else if(strncmp(cmd_line,"cd",2)==0) {
            DIR_info * dir = NULL;

            if (cmd_tokens.num_of_tokens > 1) {
                char *sub = cmd_tokens.tokens[1];
                dir = get_file_from(files, sub);
            }

            if (dir == NULL)
                printf("Error: does not exist\n");
            else if (dir->dir_attr.ATTR_DIRECTORY) {
                unsigned int FstCluster = dir->FstCluster;

                if (FstCluster == 0)
                    FstCluster = FstCluster + 2;

                unsigned int firstSector = getFirstSectorofClusterN(FstCluster, bpb_info.BPB_SecPerClus, getFirstDataSector(bpb_info));
                unsigned int dir_address = firstSector * bpb_info.BPB_BytesPerSec;

                current_directory = dir_address;

                current_dir_info = dir_constructor(buffer, dir_address, 0);
                files = get_dir_files(buffer, current_dir_info, bpb_info.BPB_SecPerClus, bpb_info.BPB_BytesPerSec, FirstDataSector, FAToffset);

                terminalStr = change_terminalStr(terminalStr, dir->DIR_Name);
            }
            else {
                printf("Error: not a directory\n");
            }
        }

        else if(strncmp(cmd_line,"ls",2)==0) {
            if (cmd_tokens.num_of_tokens == 1)
                ls_function(files);
            else if (cmd_tokens.tokens[1][0] == '.' && cmd_tokens.tokens[1][1] == 0x0A)
                ls_function(files);
            else {
                DIR_info * ls_dir = get_file_from(files, cmd_tokens.tokens[1]);

                if (ls_dir == NULL)
                    printf("Error: directory doesn't exist.\n");
                else {
                    if (ls_dir->dir_attr.ATTR_DIRECTORY) {
                        DIR_files subFiles = get_dir_files_from_pointer(buffer, ls_dir, bpb_info.BPB_SecPerClus, bpb_info.BPB_BytesPerSec, FirstDataSector, FAToffset);
//                        ls_function(subFiles);
                    } else {
                        printf("Error: not a directory.\n");
                    }
                }
            }
        }

        else if(strncmp(cmd_line,"read",4)==0) {
            if (cmd_tokens.num_of_tokens < 4) {
                printf("Error: not enough args\n");
                continue;
            }

            DIR_info * file = get_file_from(files, cmd_tokens.tokens[1]);

            if (file == NULL) {
                printf("Error: file doesn't exist.\n");
                continue;
            }

            int start = atoi(cmd_tokens.tokens[2]);
            int end = atoi(cmd_tokens.tokens[3]);

            if (start >= end) {
                printf("Error: start byte can't be greater than end byte.");
                continue;
            }

            unsigned int add = bpb_info.BPB_BytesPerSec * getFirstSectorofClusterN(file->DIR_FstClusLo, bpb_info.BPB_SecPerClus, getFirstDataSector(bpb_info));

            Clusters clusters = getAllClusters(file->FstCluster, buffer, FAToffset);

            int bytes_per_cluster = bpb_info.BPB_SecPerClus * bpb_info.BPB_BytesPerSec;

            int start_read_cluster = start / bytes_per_cluster;
            unsigned int start_read_byte = start % bytes_per_cluster;
            int end_read_cluster = end / bytes_per_cluster;
            unsigned int end_read_byte = end % bytes_per_cluster;

            if (start_read_cluster > clusters.num_of_clusters) {
                continue;
            }

            for (int i = start_read_cluster; i <= end_read_cluster; i++) {
                unsigned int cluster_address = bpb_info.BPB_BytesPerSec * getFirstSectorofClusterN(clusters.clustersArray[i], bpb_info.BPB_SecPerClus, getFirstDataSector(bpb_info));

                unsigned int index;
                unsigned int index_end;

                if (i == start_read_cluster)
                    index = start_read_byte;
                else
                    index = 0;

                if (i == end_read_cluster)
                    index_end = end_read_byte;
                else
                    index_end = bytes_per_cluster;


                while (index < index_end) {
                    if (buffer[cluster_address + index] == 0x0A && buffer[cluster_address + index + 1] == 0x00)
                        break;

                    printf("%c", buffer[cluster_address + index]);
                    index++;
                }
            }

            printf("\n");
        }

        else if (strncmp(cmd_line,"stats",5) == 0) {
            DIR_info * file = NULL;

            if (cmd_tokens.num_of_tokens > 1) {
                char *sub = cmd_tokens.tokens[1];
                file = get_file_from(files, sub);
            }

            if (file != NULL) {
//                if (file->dir_attr.ATTR_DIRECTORY)
//                TODO stats
                stats_function(file);

            }
            else
                printf("Error: file/directory does not exist\n");
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

unsigned char * read_disk (char *file_path)
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

int ls_function (DIR_files files)
{
    for (int i = 0; i < files.number_of_files; i++) {
        printf("'%s'   ", files.files[i].DIR_Name);
    }

    printf("\n");

    return 0;
}

int read_file (int start, int end, unsigned int address, char * buffer)
{
    int i = start;

    while (i < end) {
        printf("%c", buffer[address + i]);
        i++;
    }

    return 0;
}

int stats_function (DIR_info * file)
{

    if (file->dir_attr.ATTR_DIRECTORY) {
        printf("Size is 0\n");
        printf("Dir name: %s\n", file->DIR_Name);
        printf("Next cluster is 0x%x\n", file->FstCluster);
    }
    else {
        printf("Size is %d\n", file->DIR_FileSize);
        printf("Read only: %u \nHidden: %u \nattr_system: %u \nattr_archive: %u \n", file->dir_attr.ATTR_READ_ONLY, file->dir_attr.ATTR_HIDDEN, file->dir_attr.ATTR_SYSTEM, file->dir_attr.ATTR_ARCHIVE);
        printf("Next cluster is 0x%x\n", file->FstCluster);
    }
    return 0;
}


char * change_terminalStr (char * terminalStrOld, char * new_dir)
{
    if (new_dir[0] == '.' && new_dir[1] == 0x0A)
        return terminalStrOld;

    token_obj terminal_tokens = getTokens(terminalStrOld);

    int isDotDot = 0;
    int endToken = terminal_tokens.num_of_tokens - 1;

    if (new_dir[0] == '.' && new_dir[1] == '.') {
        isDotDot = 1;
        endToken--;
    }

    char * newTerminal = malloc(32 * sizeof(char));
    int strChars = 0;

    newTerminal[strChars] = '/';
    strChars++;

    for (int i = 0; i < endToken; i++) {
        for (int j = 0; terminal_tokens.tokens[i][j] != '\0'; j++) {
            newTerminal[strChars] = terminal_tokens.tokens[i][j];
            strChars++;
        }

        newTerminal[strChars] = '/';
        strChars++;
    }

    if (!isDotDot) {
        for (int i = 0 ; new_dir[i] != '\0'; i++) {
            newTerminal[strChars] = new_dir[i];
            strChars++;
        }

        newTerminal[strChars] = '/';
        strChars++;
    }

    newTerminal[strChars] = ']';
    strChars++;
    newTerminal[strChars] = '\0';

    return newTerminal;
}
