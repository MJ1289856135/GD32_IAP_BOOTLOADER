#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

caddr_t _sbrk(int incr) {
    extern char _end;   // 链接脚本定义的程序结束符
    static char* heap_end;
    char* prev_heap;

    if (!heap_end) heap_end = &_end;
    prev_heap = heap_end;
    heap_end += incr;
    return (caddr_t)prev_heap;
}

int _read(int fd, void* buf, size_t count) {
    (void)fd; (void)buf; (void)count;
    return 0;
}

int _close(int fd) {
    (void)fd;
    return -1;
}

int _fstat(int fd, struct stat* st) {
    (void)fd; (void)st;
    return 0;
}

int _isatty(int fd) {
    (void)fd;
    return 1;
}

int _lseek(int fd, int ptr, int dir) {
    (void)fd; (void)ptr; (void)dir;
    return 0;
}

int _kill(int pid, int sig) {
    (void)pid; (void)sig;
    return -1;
}

int _getpid(void) {
    return 1;
}
