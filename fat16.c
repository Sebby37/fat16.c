#include <string.h>
#include "fat16.h"

void fat_create(struct fat16 *fat, drive_t drive, u32 sectors, const char label[11])
{
    const u8 jump[] = { 0xEB, 0x3C, 0x90 };
    const char oem[] = "SEBFAT16";
    const char fs_type[] = "FAT16   ";

    // Setup the BIOS Parameter Block
    struct fat_bpb *bpb = &fat->bpb;
    memcpy(bpb->jump, jump, sizeof(jump));
    memcpy(bpb->oem, oem, 8);
    bpb->bytes_per_sector = SECTOR_SIZE;
    bpb->sectors_per_cluster = 8; // 4KB clusters with this (512 * 8) | NOTE, MS HAS A TABLE FOR PICKING THIS
    bpb->reserved_sectors = 1; // Only the boot sector is reserved
    bpb->fats = 1; // Only need 1 FAT really
    bpb->root_entries = 512; // With entries being 32 bytes, this means a root dir size of 16384 bytes or 32 sectors, thus 512 entries
    bpb->sector_count = (sectors > 65535) ? 0 : (u16)sectors; // If we have more than 65535 sectors, its stored in the large sector count
    bpb->media_type = 0xF8; // Fixed disk
    {
        // Adapted straight from Microsoft, don't blame me for tmp1/tmp2 names
        u16 root_dir_sectors = ((bpb->root_entries*32) + (bpb->bytes_per_sector-1)) / bpb->bytes_per_sector;
        u32 tmp1 = sectors - (bpb->reserved_sectors + root_dir_sectors);
        u32 tmp2 = (256*bpb->sectors_per_cluster) + bpb->fats;
        u32 fat_size = (tmp1 + (tmp2-1)) / tmp2;
        bpb->sectors_per_fat = (u16)fat_size;
    }
    bpb->sectors_per_track = 63; // Reasonalbe default
    bpb->heads = 255; // Reasonable default
    bpb->hidden_sectors = 0; // Theres no partitioning or anything on this "disk", so 0 is fine
    bpb->large_sector_count = (sectors > 65535) ? sectors : 0; // I mean its accurate regardless of if its used

    // Set up Extended Boot Record
    struct fat_ebr *ebr = &fat->ebr;
    ebr->drive = 0x80; // First fixed disk, as per BIOS INT 0x13
    ebr->reserved1 = 0x00; // Reserved, just zero
    ebr->signature = 0x29; // Should be this, indicates the next 3 entries I think
    ebr->serial = 0x43626553; // Can be whatever, so it's just SebC in ASCII (corrected for endianness)
    memcpy(ebr->label, label, 11); // User supplied! Sanitize well pretty please
    memcpy(ebr->system_id, fs_type, 8); // Take a guess
    memset(ebr->boot_code, 0x90, 448); // This disk aint bootable, just make it all NOPs (0x90)
    ebr->boot_signature = 0xAA55; // Gotta be this

    // Commit boot sector to disk
    drive_write(drive, 0, (u8*)fat);
    fat->drive = drive;

    // The first two FAT entries store special values, so do that!
    u8 fat_sector[SECTOR_SIZE];
    memset(fat_sector, '\0', SECTOR_SIZE);
    fat_sector[0] = bpb->media_type;
    fat_sector[1] = 0xFF;
    fat_sector[2] = 0xFF;
    fat_sector[3] = 0xFF;
    for (int i = 0; i < bpb->fats; i++) {
        u32 fat_first_sector = bpb->reserved_sectors + (i * bpb->sectors_per_fat);
        drive_write(drive, fat_first_sector, (u8*)fat_sector);
    }

    // Now zero-out the rest of the fat tables, reusing the previous fat_sector buffer with a little cleanup
    // Inefficient, yes, but 
    memset(fat_sector, '\0', sizeof(u16)*2);
    for (int i = 0; i < bpb->fats; i++) {
        u32 fat_first_sector = bpb->reserved_sectors + (i * bpb->sectors_per_fat);
        for (int j = 0; j < bpb->sectors_per_fat; j++)
            drive_write(drive, fat_first_sector+j+1, (u8*)fat_sector);
    }

    // Populate root directory
    struct fat_entry entries[SECTOR_SIZE/sizeof(struct fat_entry)];
    memset(entries, '\0', sizeof(entries));
    u32 root_sector = bpb->reserved_sectors + (bpb->fats * bpb->sectors_per_fat);
    u32 num_root_sectors = (bpb->root_entries*sizeof(struct fat_entry))/SECTOR_SIZE;
    for (u32 i = 0; i < num_root_sectors; i++)
        drive_write(drive, root_sector+i, (u8*)entries);
}

void fat_load(struct fat16 *fat, drive_t drive)
{
    drive_read(drive, 0, (u8*)fat);
    fat->drive = drive;
}
