#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dir_object.h"

DIR_Attr get_DIR_Attr (unsigned char attrs);
char * rename_file (char * old_name, unsigned short isDir, unsigned short isVolId);

//char * volume_name;

DIR_info dir_constructor (unsigned char * buffer, unsigned int dir_address, unsigned int parent_DIR_address)
{
    DIR_info dir_info;

    dir_info.DIR_address = dir_address;

    dir_info.parent_DIR_address = parent_DIR_address;

    dir_info.dir_attr = get_DIR_Attr(buffer[dir_address+11]);

    for (int i = 0; i < 11; i++) {
        dir_info.DIR_Name[i] = buffer[dir_address + i];
    }
    dir_info.DIR_Name[12] = '\0';

    char * new_name = rename_file(dir_info.DIR_Name, dir_info.dir_attr.ATTR_DIRECTORY, dir_info.dir_attr.ATTR_VOLUME_ID);
    strcpy(new_name, dir_info.DIR_Name);

    if (dir_info.dir_attr.ATTR_VOLUME_ID) {
        strcpy(volume_name, dir_info.DIR_Name);
    }

//    dir_info.DIR_CrtTime = convert_to_big_endian_2_byte(buffer[dir_address+14], buffer[dir_address+15]);

//    dir_info.DIR_CrtDate = convert_to_big_endian_2_byte(buffer[dir_address+16], buffer[dir_address+17]);

    dir_info.DIR_FstClusLo = convert_to_big_endian_2_byte(buffer[dir_address+26], buffer[dir_address+27]);

    dir_info.DIR_FstClusHI = convert_to_big_endian_2_byte(buffer[dir_address+20], buffer[dir_address+21]);

    dir_info.DIR_FileSize = convert_to_big_endian_4_byte(buffer[dir_address+28], buffer[dir_address+29], buffer[dir_address+30], buffer[dir_address+31]);

//    dir_info.FstCluster = (dir_info.DIR_FstClusHI << 16) + dir_info.DIR_FstClusLo;

    dir_info.FstCluster = convert_to_big_endian_4_byte(buffer[dir_address+26], buffer[dir_address+27], buffer[dir_address+20], buffer[dir_address+21]);

//    printf("cluster: %x\n", dir_info.FstCluster);

    return dir_info;
}

DIR_files get_files (unsigned char * buffer, unsigned int FirstCluster, short ATTR_VOLUME_ID, unsigned int SecPerClus, unsigned int BytesPerSec, unsigned int FirstDataSector, unsigned int FAToffset)
{
    int mallocSize = 16;
    DIR_info * files = (DIR_info *) malloc(mallocSize * sizeof(DIR_info));

    unsigned int file_index = 0, DIR_address = 0;

    if (ATTR_VOLUME_ID) {
        file_index = file_index + 32;
    }

    int num_of_files = 0;

    Clusters clusters = getAllClusters(FirstCluster, buffer, FAToffset);

    for (int i = 0; i < clusters.num_of_clusters; i++) {
        DIR_address = getFirstSectorofClusterN(clusters.clustersArray[i], SecPerClus, FirstDataSector) * BytesPerSec;
        file_index = DIR_address;

        while ((file_index - DIR_address) < 512) {
            if (buffer[file_index] == 0xE5) {
                file_index = file_index + 32;
                continue;
            }

            if (buffer[file_index] == 0x00) {
                break;
            }

            if (buffer[file_index + 11] == 0x0F) {
                file_index = file_index + 32;
                continue;
            }

            if (num_of_files >= mallocSize) {
                mallocSize = mallocSize * 2;

                files = (DIR_info *) realloc(files, mallocSize * sizeof(DIR_info));
            }

            files[num_of_files] = dir_constructor(buffer, file_index, DIR_address);

            file_index = file_index + 32;
            num_of_files++;
        }
    }

    DIR_files dir_files;
    dir_files.files = files;
    dir_files.number_of_files = num_of_files;

    return dir_files;
}

DIR_files get_dir_files (unsigned char * buffer, DIR_info dir_info, unsigned int SecPerClus, unsigned int BytesPerSec, unsigned int FirstDataSector, unsigned int FAToffset)
{
    return get_files(buffer, dir_info.FstCluster, dir_info.dir_attr.ATTR_VOLUME_ID, SecPerClus, BytesPerSec, FirstDataSector, FAToffset);
}

DIR_files get_dir_files_from_pointer (unsigned char * buffer, DIR_info * dir_info, unsigned int SecPerClus, unsigned int BytesPerSec, unsigned int FirstDataSector, unsigned int FAToffset)
{
    return get_files(buffer, dir_info->FstCluster, dir_info->dir_attr.ATTR_VOLUME_ID, SecPerClus, BytesPerSec, FirstDataSector, FAToffset);
}

char * rename_file (char * old_name, unsigned short isDir, unsigned short isVolId)
{
    char * new_name = old_name;

    int isFirstSpace, i_old = 0, i_new = 0;

    if (isDir || isVolId)
        isFirstSpace = 0;
    else
        isFirstSpace = 1;

    while (old_name[i_old] != '\0') {
        if (old_name[i_old] == ' ') {
            if (isFirstSpace) {
                new_name[i_new] = '.';
                i_new++;
                isFirstSpace = 0;
            }
        }
        else {
            new_name[i_new] = old_name[i_old];
            i_new++;
        }

        i_old++;
    }

    new_name[i_new] = '\0';

    for (int i = i_new; i < 12; i++) {
        new_name[i] = '\0';
    }

    return new_name;
}

DIR_Attr get_DIR_Attr (unsigned char attrs)
{
    DIR_Attr dir_attr;

    dir_attr.ATTR_READ_ONLY = attrs & 1;
    dir_attr.ATTR_HIDDEN = (attrs>>1) & 1;
    dir_attr.ATTR_SYSTEM = (attrs>>2) & 1;
    dir_attr.ATTR_VOLUME_ID = (attrs>>3) & 1;
    dir_attr.ATTR_DIRECTORY = (attrs>>4) & 1;
    dir_attr.ATTR_ARCHIVE = (attrs>>5) & 1;

    return dir_attr;
}

DIR_info * get_file_from (DIR_files files, char * name)
{
    for (int i = 0; i < files.number_of_files; i++) {
        if (strcmp(files.files[i].DIR_Name, name) == 0)
            return &files.files[i];
    }
    return NULL;
}

unsigned int getFirstSectorofClusterN (unsigned int clusterNum, unsigned int SecPerClus, unsigned int FirstDataSector)
{
    unsigned int FirstSectorofCluster = ((clusterNum - 2) * SecPerClus) + FirstDataSector;

    return FirstSectorofCluster;
}

Clusters getAllClusters (unsigned int firstClus, unsigned char * buffer, unsigned int FAToffset)
{
    unsigned int * allClusters = malloc (8 * sizeof(unsigned int));
    int i = 0, mallocSize = 8;

    unsigned int cluster = firstClus;

    if (firstClus == 0)
        cluster = cluster + 2;

    unsigned int offset = 0;

    allClusters[i] = cluster;
    i++;

    while (1) {
        offset = cluster * 4;

        unsigned int j = offset + FAToffset;

        cluster = convert_to_big_endian_4_byte(buffer[j], buffer[j+1], buffer[j+2], buffer[j+3]) & 0X0fffffff;

        if (cluster == 0xffffff8 || cluster == 0xfffffff)
            break;

        allClusters[i] = cluster;

        if (i == (mallocSize - 1)) {
            mallocSize = mallocSize * 2;
            allClusters = (unsigned int *) realloc(allClusters, mallocSize * sizeof(unsigned int));
        }

        i++;
    }

    Clusters clusters;

    clusters.clustersArray = allClusters;
    clusters.num_of_clusters = i;

    return clusters;
}
