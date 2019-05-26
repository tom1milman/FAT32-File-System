# FAT32-File-System
Operating Systems FAT 32 file system project

By **Tom Milman**

## Files in the Submission
1. **bpb_info.h:** Takes care of all the needed BPB_Info structs and commands.
2. **dir_object (c/h):** All the structs and commands that are connected to files/dir and their clusters. 
3. **endianess (c/h):** simple endianess commands to convert from little endian to big endian.
4. **fat32_reader.c:** The main. Has the main loop in it, and setps up all the basic information to begin working with the disk image. 
5. **makefile:** the *make* file for the project.
6. **README.md:** this file.
7. **token_object (c/h):** simple struct and command to break down char arrays into separate tokes.

## How to compile it?
Make sure all the files and the *makefile* are in the working directory. From there, type in the command line
> make

## How to run it?
After compiling and without changing the working directory, type:
> ./fat32_reader {disk image path}

Better to use relative path. For example: 
> ./fat32_reader fat32.img/fat32.img

## Challenges
There were *many* challenges while doing this project. Most of them were because of c. 

Other challenges were understanding the FAT32 specs, especially how to work with the clusters, and the FAT table.

## Outside Sources
Stackoverflow for issues with c.

The *Wikipedia* page about the *Design of the FAT file system*
