#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;

// This function is called automatically when the library is loaded
__attribute__((constructor))
static void init(void) {
    // Look up the "real" write() function address from the next library in the chain
    real_write = dlsym(RTLD_NEXT, "write");
}

// Our custom write() function that intercepts all writes
ssize_t write(int fd, const void *buf, size_t count) {
    // If this is stderr (fd=2), wrap the output in red color codes
    if (fd == 2 && count > 0) {
        // ANSI escape codes for red and reset
        const char *red   = "\033[31m";
        const char *reset = "\033[0m";

        // Write the red code, then the original message, then the reset code
        // Note: We ignore error checking for brevity
        real_write(fd, red, strlen(red));
        real_write(fd, buf, count);
        real_write(fd, reset, strlen(reset));

        // We pretend we wrote exactly `count` bytes of "payload"
        return count;
    }
    // Otherwise, it's not stderr or is empty – call the real write
    return real_write(fd, buf, count);
}
