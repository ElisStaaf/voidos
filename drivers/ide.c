// Simple PIO-based (non-DMA) IDE driver code.

#include <types.h>
#include <defs.h>
#include <param.h>
#include <memlayout.h>
#include <mmu.h>
#include <proc.h>
#include <x86.h>
#include <traps.h>
#include <spinlock.h>
#include <fs.h>
#include <buf.h>

#define ATA_DATA                0x00
#define ATA_ERROR               0x01
#define ATA_PRECOMP             0x01
#define ATA_CTRL                0x02
#define ATA_SECCNT              0x02
#define ATA_SECTOR              0x03
#define ATA_CYL_LO              0x04
#define ATA_CYL_HI              0x05
#define ATA_SDH                 0x06
#define ATA_COMMAND             0x07
#define ATA_STATUS              0x07

#define ATA_SR_BSY              0x80    // Busy
#define ATA_SR_DRDY             0x40    // Drive ready
#define ATA_SR_DF               0x20    // Drive write fault
#define ATA_SR_DSC              0x10    // Drive seek complete
#define ATA_SR_DRQ              0x08    // Data request ready
#define ATA_SR_CORR             0x04    // Corrected data
#define ATA_SR_IDX              0x02    // Inlex
#define ATA_SR_ERR              0x01    // Error

#define ATA_CMD_READ            0x20
#define ATA_CMD_WRITE           0x30

#define IO_BASE0                0x1F0   // Primary
#define IO_BASE1                0x170   // Secondary
#define IO_CTRL0                0x3F4
#define IO_CTRL1                0x374

#define FS_DEVNO                1       // Primary slave
#define SWAP_DEVNO				2       // Secondary master
#define SECTSIZE				512

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;
static struct buf *idequeue;

static int havedisk1 = 0;
static int havedisk2 = 0;
static void idestart(struct buf*);

// Wait for IDE disk to become ready.
static int
idewait(unsigned short iobase, int check_error) {
    int r;
    while ((r = inb(iobase + ATA_STATUS)) & ATA_SR_BSY);
    if (check_error && (r & (ATA_SR_DF | ATA_SR_ERR)) != 0) {
        return -1;
    }
    return 0;
}

void
ideinit(void)
{
  int i;

  initlock(&idelock, "ide");
  picenable(IRQ_IDE);
  ioapicenable(IRQ_IDE, ncpu - 1);
  idewait(IO_BASE0, 0);

  // Check if disk 1 is present
  outb(IO_BASE0 + ATA_SDH, 0xe0 | ((FS_DEVNO & 1)<<4));
  for(i=0; i<1000; i++){
    if(inb(IO_BASE0 + ATA_STATUS) != 0){
      havedisk1 = 1;
      break;
    }
  }
  if (havedisk1 == 0)
    panic("ide disk 1 not present");

  idewait(IO_BASE1, 0);
  // check if disk 2 is present
  outb(IO_BASE1 + ATA_SDH, 0xe0 | ((SWAP_DEVNO & 1)<<4));
  for(i=0; i<1000; i++){
    if(inb(IO_BASE1 + ATA_STATUS) != 0){
      havedisk2 = 1;
      break;
    }
  }
  if (havedisk2 == 0)
    panic("ide disk 2(swap disk) not present");

  // Switch back to disk 0.
  outb(0x1f6, 0xe0 | (0<<4));
}

// Start the request for b.  Caller must hold idelock.
static void
idestart(struct buf *b)
{
  if(b == 0)
    panic("idestart");
  if(b->blockno >= FSSIZE)
    panic("incorrect blockno");
  int sector_per_block =  BSIZE/SECTSIZE;
  int sector = b->blockno * sector_per_block;

  if (sector_per_block > 7) panic("idestart");

  idewait(IO_BASE0, 0);
  outb(0x3f6, 0);  // generate interrupt
  outb(0x1f2, sector_per_block);  // number of sectors
  outb(0x1f3, sector & 0xff);
  outb(0x1f4, (sector >> 8) & 0xff);
  outb(0x1f5, (sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
  if(b->flags & B_DIRTY){
    outb(0x1f7, ATA_CMD_WRITE);
    outsl(0x1f0, b->data, BSIZE/4);
  } else {
    outb(0x1f7, ATA_CMD_READ);
  }
}

// Interrupt handler.
void
ideintr(void)
{
  struct buf *b;

  // First queued buffer is the active request.
  acquire(&idelock);
  if((b = idequeue) == 0){
    release(&idelock);
    // cprintf("spurious IDE interrupt\n");
    return;
  }
  idequeue = b->qnext;

  // Read data if needed.
  if(!(b->flags & B_DIRTY) &&   idewait(IO_BASE0, 1) >= 0)
    insl(0x1f0, b->data, BSIZE/4);

  // Wake process waiting for this buf.
  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;
  wakeup(b);

  // Start disk on next buf in queue.
  if(idequeue != 0)
    idestart(idequeue);

  release(&idelock);
}

//PAGEBREAK!
// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void
iderw(struct buf *b)
{
  struct buf **pp;

  if(!(b->flags & B_BUSY))
    panic("iderw: buf not busy");
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  if(b->dev != 0 && !havedisk1)
    panic("iderw: ide disk 1 not present");

  acquire(&idelock);  //DOC:acquire-lock

  // Append b to idequeue.
  b->qnext = 0;
  for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
    ;
  *pp = b;

  // Start disk if necessary.
  if(idequeue == b)
    idestart(b);

  // Wait for request to finish.
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
    sleep(b, &idelock);
  }

  release(&idelock);
}

int
read_swap(uint secno, void *dst)
{
    acquire(&idelock);

	uint ideno = SWAP_DEVNO, iobase = IO_BASE1, ioctrl = IO_CTRL1;
    uint nsecs = 8;
    idewait(IO_BASE1, 0);

    outb(ioctrl + ATA_CTRL, 0);
    outb(iobase + ATA_SECCNT, nsecs);
    outb(iobase + ATA_SECTOR, secno & 0xFF);
    outb(iobase + ATA_CYL_LO, (secno >> 8) & 0xFF);
    outb(iobase + ATA_CYL_HI, (secno >> 16) & 0xFF);
    outb(iobase + ATA_SDH, 0xE0 | ((ideno & 1) << 4) | ((secno >> 24) & 0xF));
    outb(iobase + ATA_COMMAND, ATA_CMD_READ);

    int ret = 0;
    for (; nsecs > 0; nsecs --, dst += SECTSIZE) {
        if ((ret = idewait(iobase, 1)) != 0) {
            release(&idelock);
            return ret;
        }
        insl(iobase, dst, SECTSIZE / 4);
    }

    release(&idelock);
    return 0;
}

int
write_swap(uint secno, const void *src)
{
    acquire(&idelock);

	uint ideno = SWAP_DEVNO, iobase = IO_BASE1, ioctrl = IO_CTRL1;
    uint nsecs = 8;
    idewait(IO_BASE1, 0);

	outb(ioctrl + ATA_CTRL, 0);
    outb(iobase + ATA_SECCNT, nsecs);
    outb(iobase + ATA_SECTOR, secno & 0xFF);
    outb(iobase + ATA_CYL_LO, (secno >> 8) & 0xFF);
    outb(iobase + ATA_CYL_HI, (secno >> 16) & 0xFF);
    outb(iobase + ATA_SDH, 0xE0 | ((ideno & 1) << 4) | ((secno >> 24) & 0xF));
    outb(iobase + ATA_COMMAND, ATA_CMD_WRITE);

    int ret = 0;
    for (; nsecs > 0; nsecs --, src += SECTSIZE) {
        if ((ret = idewait(iobase, 1)) != 0) {
            release(&idelock);
            return ret;
        }
        outsl(iobase, src, SECTSIZE / 4);
    }

    release(&idelock);
    return 0;
}
