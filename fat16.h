#pragma once
#include "defines.h"
#include "drive.h"

// BIOS Parameter Block
struct fat_bpb {
    u8 jump[3]; // Should always be EB 3C 90 (JMP SHORT 3C NOP)
    char oem[8];  // OEM identifier, padded with spaces
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    u8 fats;
    u16 root_entries;
    u16 sector_count;
    u8 media_type;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 heads;
    u32 hidden_sectors;
    u32 large_sector_count;
} __attribute__((packed));

// Extended Boot Record
struct fat_ebr {
    u8 drive;
    u8 reserved1;
    u8 signature;
    u32 serial;
    char label[11];
    char system_id[8];
    u8 boot_code[448];
    u16 boot_signature;
} __attribute__((packed));

// 8.3 Directory Entry Format
struct fat_entry {
    union {
        char fullname[11];
        struct {
            char name[8];
            char ext[3];
        };
        u8 status;
    } name;
    u8 attributes;  // READ_ONLY=0x01 HIDDEN=0x02 SYSTEM=0x04 VOLUME_ID=0x08 DIRECTORY=0x10 ARCHIVE=0x20 LFN=READ_ONLY|HIDDEN|SYSTEM|VOLUME_ID
    u8 reserved1;
    u8 creation_hundredths;  // A resolution of 10ms
    u16 creation_time;  // <15:11>=Hours, <10:5>=Minutes, <4:0>=Seconds/2
    u16 creation_date;  // <15:9>=Year(0=1980), <8:5>=Month, <4:0>=Day
    u16 accessed_date;  //  Same date format
    u16 cluster_high;   // Always 0 for FAT16
    u16 modified_time;  //  Same time format
    u16 modified_date;  //  Same date format
    u16 cluster_low;    // Actually points to the first cluster of the entry
    u32 size;           // File size in bytes, meaningless for directories
};

// The FAT16 boot sector
struct fat16 {
    // The drive's BIOS Parameter Block (BPB)
    struct fat_bpb bpb;

    // The drive's Extended Boot Record (EBR)
    struct fat_ebr ebr;

    // The physical file/drive that the disk is actually stored on
    drive_t drive;
} __attribute__((packed));

// Creates a new FAT16 drive on a given driver file descriptor
void fat_create(struct fat16 *fat, drive_t drive, u32 sectors, const char label[11]);

// Loads a FAT16 drive from a given file descriptor
void fat_load(struct fat16 *fat, drive_t drive);

// What internal functions should we need for working with a fat fs?
// == Raw Cluster Interface ==
// ReadCluster(n, buf)
// WriteCluster(n, buf)
//

// What other functions should a fat fs expose?
// == File Entry Operations ==
// Create(path, permissions)
// Delete(path)
// Rename(path, new_path)
// Stat(path)  // Can also return like -1 if an entry doesn't exist
// == File Content Operations ==
// Read(path, buf, n, offset)
// Write(path, buf, len, offset)
// == Directory Operations ==
// MKDir(path, permissions)
// RMDir(path)
// ListDir(path, entries_out, n_entries_out, max_entries)