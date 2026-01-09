#pragma once

/*
This is an emulation of a HDD interface over files.
You can create a drive given a number of sectors, then read/write in sectors.
*/

#include <fcntl.h>
#include <unistd.h>
#include "defines.h"

#define SECTOR_SIZE 512

typedef int drive_t;

// Create a new drive on a given file descriptor with `sectors` sectors
static inline drive_t drive_create(int fd, u32 sectors)
{
    ftruncate(fd, sectors*SECTOR_SIZE);
    return fd;
}

// Read a sector `sector` into `buf` from the disk
static inline void drive_read(drive_t drive, u32 sector, u8 buf[SECTOR_SIZE])
{
    lseek((int)drive, sector*SECTOR_SIZE, SEEK_SET);
    read((int)drive, buf, SECTOR_SIZE);
}

// Write `buf` to sector `sector` on the disk
static inline void drive_write(drive_t drive, u32 sector, u8 buf[SECTOR_SIZE])
{
    lseek((int)drive, sector*SECTOR_SIZE, SEEK_SET);
    write((int)drive, buf, SECTOR_SIZE);
}