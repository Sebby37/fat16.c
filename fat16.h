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