#include "endianess.h"

unsigned int convert_to_big_endian_4_byte (unsigned char byte_0, unsigned char byte_1, unsigned char byte_2, unsigned char byte_3)
{
    unsigned int big_endian = 0;

    big_endian = (byte_3 << 24) & 0xff000000 |
                 (byte_2 << 16) & 0xff0000 |
                 (byte_1 << 8) & 0xff00 |
                 byte_0 & 0xff;

    return big_endian;
}

unsigned int convert_to_big_endian_3_byte (unsigned char byte_0, unsigned char byte_1, unsigned char byte_2)
{
    unsigned int big_endian = 0;

    big_endian = (byte_2 << 16) & 0xff0000 |
                 (byte_1 << 8) & 0xff00 |
                 byte_0 & 0xff;

    return big_endian;
}

unsigned int convert_to_big_endian_2_byte (unsigned char byte_0, unsigned char byte_1)
{
    unsigned int big_endian = 0;

    big_endian = (byte_1 << 8) & 0xff00 |
                 byte_0 & 0xff;

    return big_endian;
}

unsigned int convert_to_big_endian_1_byte (unsigned char byte_0)
{
    unsigned int big_endian = 0;

    big_endian = byte_0 & 0xff;

    return big_endian;
}
