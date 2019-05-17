CC=gcc

fat32_reader_make: fat32_reader.c
	$(CC) -o fat32_reader fat32_reader.c dir_object.c endianess.c token_object.c -I.
