#include "endianess.h"

typedef struct BPB_Info
{
    unsigned int BPB_BytesPerSec;
    unsigned int BPB_SecPerClus;
    unsigned int BPB_RsvdSecCnt;
    unsigned int BPB_NumFATs;
    unsigned int BPB_FATSz32;
    unsigned int BPB_TotSec32;

}BPB_Info;

BPB_Info get_BPB_info (unsigned char * buffer)
{
    BPB_Info info;

    info.BPB_BytesPerSec = convert_to_big_endian_2_byte(buffer[11], buffer[12]);

    info.BPB_SecPerClus = convert_to_big_endian_1_byte(buffer[13]);

    info.BPB_RsvdSecCnt = convert_to_big_endian_2_byte(buffer[14], buffer[15]);

    info.BPB_NumFATs = convert_to_big_endian_1_byte(buffer[16]);

    info.BPB_FATSz32 = convert_to_big_endian_4_byte(buffer[36], buffer[37], buffer[38], buffer[39]);

    info.BPB_TotSec32 = convert_to_big_endian_4_byte(buffer[32], buffer[33], buffer[34], buffer[35]);

    return info;
}

int print_BPB_info (BPB_Info * info)
{
    printf("BPB_BytesPerSec is 0x%X, %d \n", info->BPB_BytesPerSec, info->BPB_BytesPerSec);
    printf("BPB_SecPerClus is 0x%X, %d \n", info->BPB_SecPerClus, info->BPB_SecPerClus);
    printf("BPB_RsvdSecCnt is 0x%X, %d \n", info->BPB_RsvdSecCnt, info->BPB_RsvdSecCnt);
    printf("BPB_NumFATs is 0x%X, %d \n", info->BPB_NumFATs, info->BPB_NumFATs);
    printf("BPB_FATSz32 is 0x%X, %d \n", info->BPB_FATSz32, info->BPB_FATSz32);

    return 0;
}