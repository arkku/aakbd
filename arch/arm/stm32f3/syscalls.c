/*
 * syscalls.c: Newlib syscall stubs for ARM.
 *
 * Copyright 2026 Kimmo Kulovesi
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;
extern char _end;

void *
_sbrk (int incr) {
    static char *heap_end;
    char *prev;
    if (!heap_end) {
        heap_end = &_end;
    }
    prev = heap_end;
    heap_end += incr;
    return (void *) prev;
}

int
_fstat (int fd, struct stat *st) {
    if (fd < 0) {
        errno = EBADF;
        return -1;
    }
    st->st_mode = S_IFCHR;
    return 0;
}

int
_isatty (int fd) {
    return fd == 0 || fd == 1 || fd == 2;
}

int
_close (int fd) {
    (void) fd;
    return -1;
}

int
_read (int fd, char *ptr, int len) {
    (void) fd;
    (void) ptr;
    return 0;
}

int
_lseek (int fd, int ptr, int dir) {
    (void) fd;
    (void) ptr;
    (void) dir;
    return -1;
}

int
_write (int fd, const char *ptr, int len) {
    (void) fd;
    return len;
}
