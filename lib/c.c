/* minimal libc implementation for shecc */

#define NULL 0

#if defined(__arm__)
#define __syscall_exit 1
#define __syscall_read 3
#define __syscall_write 4
#define __syscall_close 6
#define __syscall_open 5
#define __syscall_brk 45
#define __syscall_mmap2 192
#define __syscall_munmap 91

#elif defined(__riscv)
#define __syscall_exit 93
#define __syscall_read 63
#define __syscall_write 64
#define __syscall_close 57
#define __syscall_open 1024
#define __syscall_openat 56
#define __syscall_brk 214
#define __syscall_mmap2 222
#define __syscall_munmap 215

#endif

#define INT_BUF_LEN 16

typedef int FILE;

void abort();

int strlen(char *str)
{
    int i = 0;
    while (str[i])
        i++;
    return i;
}

int strcmp(char *s1, char *s2)
{
    int i = 0;
    while (s1[i] && s2[i]) {
        if (s1[i] < s2[i])
            return -1;
        else if (s1[i] > s2[i])
            return 1;
        i++;
    }
    return s1[i] - s2[i];
}

int strncmp(char *s1, char *s2, int len)
{
    int i = 0;
    while (i < len) {
        if (s1[i] < s2[i])
            return -1;
        else if (s1[i] > s2[i])
            return 1;
        i++;
    }
    return 0;
}

char *strcpy(char *dest, char *src)
{
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

char *strncpy(char *dest, char *src, int len)
{
    int i = 0;
    int beyond = 0;
    while (i < len) {
        if (beyond == 0) {
            dest[i] = src[i];
            if (src[i] == 0)
                beyond = 1;
        } else {
            dest[i] = 0;
        }
        i++;
    }
    return dest;
}

/* set 10 digits (32bit) without div */
void __str_base10(char *pb, int val)
{
    int neg = 0;

    if (val == -2147483648) {
        strncpy(pb + INT_BUF_LEN - 11, "-2147483648", 11);
        return;
    }
    if (val < 0) {
        neg = 1;
        val = -val;
    }

    while (val >= 1000000000) {
        val -= 1000000000;
        pb[INT_BUF_LEN - 10]++;
    }
    while (val >= 100000000) {
        val -= 100000000;
        pb[INT_BUF_LEN - 9]++;
    }
    while (val >= 10000000) {
        val -= 10000000;
        pb[INT_BUF_LEN - 8]++;
    }
    while (val >= 1000000) {
        val -= 1000000;
        pb[INT_BUF_LEN - 7]++;
    }
    while (val >= 100000) {
        val -= 100000;
        pb[INT_BUF_LEN - 6]++;
    }
    while (val >= 10000) {
        val -= 10000;
        pb[INT_BUF_LEN - 5]++;
    }
    while (val >= 1000) {
        val -= 1000;
        pb[INT_BUF_LEN - 4]++;
    }
    while (val >= 100) {
        val -= 100;
        pb[INT_BUF_LEN - 3]++;
    }
    while (val >= 10) {
        val -= 10;
        pb[INT_BUF_LEN - 2]++;
    }
    while (val >= 1) {
        val -= 1;
        pb[INT_BUF_LEN - 1]++;
    }

    if (neg == 1) {
        int c = 0;
        while (pb[c] == '0')
            c++;
        if (c > 0)
            pb[c - 1] = '-';
    }
}

void __str_base8(char *pb, int val)
{
    int c = INT_BUF_LEN - 1;
    while (c > 0) {
        int v = val & 0x7;
        pb[c] = '0' + v;
        val = val >> 3;
        c--;
    }
}

void __str_base16(char *pb, int val)
{
    int c = INT_BUF_LEN - 1;
    while (c > 0) {
        int v = val & 0xf;
        if (v < 10)
            pb[c] = '0' + v;
        else if (v < 16)
            pb[c] = 'a' + v - 10;
        else
            abort();
        val = val >> 4;
        c--;
    }
}

int __format(char *buffer,
             int val,
             int width,
             int zeropad,
             int base,
             int alternate_form)
{
    int bi = 0;
    char pb[INT_BUF_LEN];
    int pbi = 0;

    if (alternate_form == 1) {
        switch (base) {
        case 8:
            /* octal */
            buffer[0] = '0';
            bi = 1;
            width -= 1;
            break;
        case 16:
            /* hex */
            buffer[0] = '0';
            buffer[1] = 'x';
            bi = 2;
            width -= 2;
            break;
        default:
            /* decimal */
            /* do nothing */
            break;
        }
        if (width < 0)
            width = 0;
    }

    /* set to zeroes */
    while (pbi < INT_BUF_LEN) {
        pb[pbi] = '0';
        pbi++;
    }

    switch (base) {
    case 8:
        __str_base8(pb, val);
        break;
    case 10:
        __str_base10(pb, val);
        break;
    case 16:
        __str_base16(pb, val);
        break;
    default:
        abort();
    }

    while (width > INT_BUF_LEN) {
        /* need to add extra padding */
        if (zeropad == 1)
            buffer[bi] = '0';
        else
            buffer[bi] = ' ';
        bi++;
        width--;
    }

    /* no padding */
    if (width == 0) {
        int c = 0;
        int started = 0;

        /* output from first digit */
        while (c < INT_BUF_LEN) {
            if (pb[c] != '0')
                started = 1;
            if (started) {
                buffer[bi] = pb[c];
                bi++;
            }
            c++;
        }
        /* special case - zero */
        if (started == 0) {
            buffer[bi] = '0';
            bi++;
        }
    } else {
        /* padding */
        int c = INT_BUF_LEN - width;
        int started = 0;
        while (c < INT_BUF_LEN) {
            if (pb[c] != '0')
                started = 1;
            if (started)
                buffer[bi] = pb[c];
            else if (zeropad == 1)
                buffer[bi] = '0';
            else
                buffer[bi] = ' ';
            bi++;
            c++;
        }
    }
    return bi;
}

void printf(char *str, ...)
{
    int *var_args = &str - 4;
    char buffer[200];
    int si = 0, bi = 0, pi = 0;

    while (str[si]) {
        if (str[si] != '%') {
            buffer[bi] = str[si];
            bi++;
            si++;
        } else {
            int w = 0, zp = 0, pp = 0;

            si++;
            if (str[si] == '#') {
                pp = 1;
                si++;
            }
            if (str[si] == '0') {
                zp = 1;
                si++;
            }
            if (str[si] >= '1' && str[si] <= '9') {
                w = str[si] - '0';
                si++;
                while (str[si] >= '0' && str[si] <= '9') {
                    w = w * 10;
                    w += str[si] - '0';
                    si++;
                }
            }
            if (str[si] == 's') {
                /* append param pi as string */
                int l = strlen(var_args[pi]);
                strcpy(buffer + bi, var_args[pi]);
                bi += l;
            } else if (str[si] == 'c') {
                /* append param pi as char */
                buffer[bi] = var_args[pi];
                bi += 1;
            } else if (str[si] == 'o') {
                /* append param as octal */
                int v = var_args[pi];
                bi += __format(buffer + bi, v, w, zp, 8, pp);
            } else if (str[si] == 'd') {
                /* append param as decimal */
                int v = var_args[pi];
                bi += __format(buffer + bi, v, w, zp, 10, 0);
            } else if (str[si] == 'x') {
                /* append param as hex */
                int v = var_args[pi];
                bi += __format(buffer + bi, v, w, zp, 16, pp);
            } else if (str[si] == '%') {
                /* append literal '%' character */
                buffer[bi] = '%';
                bi++;
                pi++;
            }
            pi--;
            si++;
        }
    }
    buffer[bi] = 0;
    __syscall(__syscall_write, 1, buffer, bi);
}

char *memcpy(char *dest, char *src, int count)
{
    if (count > 0) {
        do {
            count--;
            dest[count] = src[count];
        } while (count > 0);
    }
    return dest;
}

void exit(int exit_code)
{
    __syscall(__syscall_exit, exit_code);
}

void abort()
{
    printf("Abnormal program termination\n");
    exit(-1);
}

FILE *fopen(char *filename, char *mode)
{
    if (!strcmp(mode, "wb"))
#if defined(__arm__)
        return __syscall(__syscall_open, filename, 65, 0x1fd);
#elif defined(__riscv)
        /* FIXME: mode not work currently in RISC-V */
        return __syscall(__syscall_openat, -100, filename, 65, 0x1fd);
#endif
    if (!strcmp(mode, "rb"))
#if defined(__arm__)
        return __syscall(__syscall_open, filename, 0, 0);
#elif defined(__riscv)
        return __syscall(__syscall_openat, -100, filename, 0, 0);
#endif

    abort();
}

int fclose(FILE *stream)
{
    __syscall(__syscall_close, stream);
    return 0;
}

int fgetc(FILE *stream)
{
    char buf;
    int r = __syscall(__syscall_read, stream, &buf, 1);
    if (r <= 0)
        return -1;
    return buf;
}

char *fgets(char *str, int n, FILE *stream)
{
    int i = 0;
    do {
        char c = fgetc(stream);
        if (c == -1 || c == 255) {
            if (i == 0)
                /* EOF on first char */
                return NULL;
            /* EOF in the middle */
            str[i] = 0;
            return str;
        }
        str[i] = c;
        i++;
    } while (str[i - 1] != '\n');
    str[i] = 0;
    return str;
}

int fputc(int c, FILE *stream)
{
    char buf[1];
    buf[0] = c;
    __syscall(__syscall_write, stream, &buf, 1);
    return 0;
}

#define PAGESIZE 4096

typedef struct chunk {
    struct _chunk *next;
    struct _chunk *prev;
    int size;
    void *ptr;
} chunk_t;

chunk_t *head;

int align_up(int size)
{
    int mask = PAGESIZE - 1;

    return ((size - 1) | mask) + 1;
}

void *malloc(int size)
{
    if (size <= 0)
        return NULL;

    int flags = 34; /* MAP_PRIVATE (0x02) | MAP_ANONYMOUS (0x20) */
    int prot = 3;   /* PROT_READ (0x01) | PROT_WRITE (0x02) */

    if (head == NULL) {
        char *tmp = __syscall(__syscall_mmap2, 0, align_up(sizeof(chunk_t)),
                              prot, flags, -1, 0);
        head = tmp;
        head->next = NULL;
        head->prev = NULL;
        head->ptr = NULL;
        head->size = 0;
    }

    chunk_t *cur = head;

    while (cur->next != NULL)
        cur = cur->next;

    chunk_t *new =
        __syscall(__syscall_mmap2, NULL, align_up(sizeof(chunk_t) + size), prot,
                  flags, -1, 0);
    cur->next = new;
    new->prev = cur;

    cur = new;
    cur->next = NULL;
    cur->size = size;
    cur->ptr = new + sizeof(chunk_t) - 4;
    return cur->ptr;
}

void free(void *ptr)
{
    if (ptr == NULL)
        return;

    chunk_t *cur = head;

    while (cur->ptr != ptr)
        cur = cur->next;

    chunk_t *prev;
    if (cur->prev != NULL) {
        prev = cur->prev;
        prev->next = cur->next;
    } else
        head = cur->next;

    chunk_t *next;
    if (cur->next != NULL) {
        next = cur->next;
        next->prev = cur->prev;
    } else
        prev->next = NULL;

    __syscall(__syscall_munmap, cur->ptr, cur->size);
    __syscall(__syscall_munmap, cur, sizeof(chunk_t) - 4);
}