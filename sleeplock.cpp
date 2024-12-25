// Sleeping locks

#include "types.hpp"
#include "defs.hpp"
#include "param.hpp"
#include "x86.hpp"
#include "memlayout.hpp"
#include "mmu.hpp"
#include "proc.hpp"
#include "spinlock.hpp"
#include "sleeplock.hpp"

void initsleeplock(struct sleeplock* lk, const char* name) {
    initlock(&lk->lk, "sleep lock");
    lk->name   = name;
    lk->locked = 0;
    lk->pid    = 0;
}

void acquiresleep(struct sleeplock* lk) {
    acquire(&lk->lk);
    while (lk->locked) {
        sleep(lk, &lk->lk);
    }
    lk->locked = 1;
    lk->pid    = myproc()->pid;
    release(&lk->lk);
}

void releasesleep(struct sleeplock* lk) {
    acquire(&lk->lk);
    lk->locked = 0;
    lk->pid    = 0;
    wakeup(lk);
    release(&lk->lk);
}

int holdingsleep(struct sleeplock* lk) {
    int r;

    acquire(&lk->lk);
    r = lk->locked && (lk->pid == myproc()->pid);
    release(&lk->lk);
    return r;
}
