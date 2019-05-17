#include "endianess.h"

typedef struct DIR_Attr
{
    short ATTR_READ_ONLY;
    short ATTR_HIDDEN;
    short ATTR_SYSTEM;
    short ATTR_VOLUME_ID;
    short ATTR_DIRECTORY;
    short ATTR_ARCHIVE;
}DIR_Attr;

typedef struct DIR_info
{
    unsigned int DIR_address;
    unsigned int parent_DIR_address;
    unsigned char DIR_Name [12] ;
//    unsigned int DIR_CrtTime;
//    unsigned int DIR_CrtDate;
    unsigned int DIR_FstClusLo;
    unsigned int DIR_FstClusHI;
    unsigned int FstCluster;
    unsigned int DIR_FileSize;
    int number_of_files;
    DIR_Attr dir_attr;

}DIR_info;

typedef struct DIR_files
{
    DIR_info * files;
    int number_of_files;
}DIR_files;

typedef struct Clusters
{
    unsigned int * clustersArray;
    int num_of_clusters;
}Clusters;

DIR_info dir_constructor (unsigned char * buffer, unsigned int dir_address, unsigned int parent_DIR_address);
DIR_files get_dir_files (unsigned char * buffer, DIR_info dir_info);
DIR_files get_dir_files_from_pointer (unsigned char * buffer, DIR_info * dir_info);
DIR_info * get_file_from (DIR_files files, char * name);
unsigned int getFirstSectorofClusterN (unsigned int clusterNum, unsigned int SecPerClus, unsigned int FirstDataSector);
Clusters getAllClusters (unsigned int firstClus, unsigned char * buffer, unsigned int FAToffset);
