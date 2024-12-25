#include "syscall.hpp"
#include "traps.hpp"

// Boilerplate wrapper for the system call definition - `SYS_ ## name` is
// expanded into the system name macro, which in turn gets replaced with an
// integer literal - system call code. This allows keeping the
// implementation and the system library in sync. `$12` is the syntax for
// immediate integer literal operand
#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

// Common implementation Implementation of the system call functions
SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
