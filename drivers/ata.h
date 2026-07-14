#ifndef ATA_H
#define ATA_H

#include "../include/types.h"
#include "io.h"

/* ============================= */
/* ATA Primary Bus I/O Registers */
/* ============================= */

#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERROR        0x1F1
#define ATA_PRIMARY_FEATURES     0x1F1
#define ATA_PRIMARY_SECCOUNT0    0x1F2
#define ATA_PRIMARY_LBA0         0x1F3
#define ATA_PRIMARY_LBA1         0x1F4
#define ATA_PRIMARY_LBA2         0x1F5
#define ATA_PRIMARY_HDDEVSEL     0x1F6
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_STATUS       0x1F7

#define ATA_PRIMARY_CONTROL      0x3F6
#define ATA_PRIMARY_ALTSTATUS    0x3F6

/* ============================= */
/* ATA Commands                  */
/* ============================= */

#define ATA_CMD_READ_SECTORS     0x20
#define ATA_CMD_WRITE_SECTORS    0x30
#define ATA_CMD_CACHE_FLUSH      0xE7
#define ATA_CMD_IDENTIFY         0xEC

/* ============================= */
/* ATA Status Register Bits      */
/* ============================= */

#define ATA_SR_ERR   0x01
#define ATA_SR_DRQ   0x08
#define ATA_SR_DF    0x20
#define ATA_SR_DRDY  0x40
#define ATA_SR_BSY   0x80

/* ============================= */
/* Drive Selection               */
/* ============================= */

#define ATA_MASTER 0xE0
#define ATA_SLAVE  0xF0

/* ============================= */
/* Sector Size                   */
/* ============================= */

#define ATA_SECTOR_SIZE 512

/* ============================= */
/* Driver Functions              */
/* ============================= */

/* Initialize the ATA driver */
void ata_init(void);

/* Detect if a drive is present */
bool ata_detect(void);

/* Wait until the drive is ready */
bool ata_wait(void);

/* Read one 512-byte sector */
bool ata_read_sector(uint32_t lba, uint8_t *buffer);

/* Write one 512-byte sector */
bool ata_write_sector(uint32_t lba, const uint8_t *buffer);

/* Flush the drive cache */
void ata_flush(void);

#endif
