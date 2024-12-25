# Is sheet 01 (after the TOC) a left sheet or a right sheet?
sheet1: left

# "left" and "right" specify which page of a two-page spread a file
# must start on.  "left" means that a file must start on the first of
# the two pages.  "right" means it must start on the second of the two
# pages.  The file may start in either column.
#
# "even" and "odd" specify which column a file must start on.  "even"
# means it must start in the left of the two columns (00).  "odd" means it
# must start in the right of the two columns (50).
#
# You'd think these would be the other way around.

# types.hpp either
# param.hpp either
# defs.hpp either
# x86.hpp either
# asm.hpp either
# mmu.hpp either
# elf.hpp either
# mp.hpp either

even: entry.S      # mild preference
even: entryother.S # mild preference
even: main.cpp
# mp.cpp don't care at all
# even: initcode.S
# odd: init.cpp

left: spinlock.hpp
even: spinlock.hpp

# This gets struct proc and allocproc on the same spread
left: proc.hpp
even: proc.hpp

# goal is to have two action-packed 2-page spreads,
# one with
#     userinit growproc fork exit wait
# and another with
#     scheduler sched yield forkret sleep wakeup1 wakeup
right: proc.cpp # VERY important
even: proc.cpp  # VERY important

# A few more action packed spreads
# page table creation and process loading
#     walkpgdir mappages setupkvm switch[ku]vm inituvm (loaduvm)
# process memory management
#     allocuvm deallocuvm freevm
left: vm.cpp

even: kalloc.cpp # mild preference

# syscall.hpp either
# trapasm.S either
# traps.hpp either
# even: trap.cpp
# vectors.pl either
# syscall.cpp either
# sysproc.cpp either

# buf.hpp either
# dev.hpp either
# fcntl.hpp either
# stat.hpp either
# file.hpp either
# fs.hpp either
# fsvar.hpp either
# left: ide.cpp # mild preference
even: ide.cpp
# odd: bio.cpp

# log.cpp fits nicely in a spread
even: log.cpp
left: log.cpp

# with fs.cpp starting on 2nd column of a left page, we get these 2-page spreads:
#	ialloc iupdate iget idup ilock iunlock iput iunlockput
#	bmap itrunc stati readi writei
#	namecmp dirlookup dirlink skipelem namex namei
#	fileinit filealloc filedup fileclose filestat fileread filewrite
# starting on 2nd column of a right page is not terrible either
odd: fs.cpp  # VERY important
left: fs.cpp # mild preference
# file.cpp either
# exec.cpp either
# sysfile.cpp either

# Mild preference, but makes spreads of mp.cpp, lapic.cpp, and ioapic.cpp+picirq.cpp
even: mp.cpp
left: mp.cpp

# even: pipe.cpp  # mild preference
# string.cpp either
# left: kbd.hpp  # mild preference
even: kbd.hpp
even: console.cpp
odd: sh.cpp

even: bootasm.S    # mild preference
even: bootmain.cpp # mild preference
