// Create a zombie process that
// must be reparented at exit.

#include "types.hpp"
#include "stat.hpp"
#include "user.hpp"

int main(void) {
    if (fork() > 0) {
        sleep(5); // Let child exit before parent.
    }
    exit();
}
