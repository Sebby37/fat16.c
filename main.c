#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat16.h"

// I'd like to turn this into a basic shell of sorts, with touch, ls, mkdir, etc.

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: ./fat16 MODE PATH [options...]\n");
        printf("\tMODE\tOne of (create, read, write)\n");
        printf("\tPATH\tPath to the emulated disk\n");
        printf("\toptions\tMode-specific options\n");
        return 1;
    }

    char *mode = argv[1];
    char *path = argv[2];

    if (strncmp(mode, "create", 7) == 0) {
        if (argc < 5) {
            printf("Not enough args! Specify the number of sectors and the volume label\n");
            return 1;
        }

        u32 sectors = atoi(argv[3]);
        int drive = creat(path, 0644);
        drive_create(drive, sectors);

        char label[11];
        size_t label_len = strlen(argv[4]);
        strncpy(label, argv[4], 11);  // Apparently strncpy fills the rest of a buffer with NULLs, fun!
        memset(&label[label_len], ' ', sizeof(label)-label_len); // So fill the rest with spaces!

        struct fat16 fat;
        fat_create(&fat, drive, sectors, label);

        printf("Created a new FAT volume on a new drive '%s' that is %d bytes in size, named '%s'\n", path, sectors*SECTOR_SIZE, argv[4]);
    }

    return 0;
}