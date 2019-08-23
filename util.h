/* See LICENSE file for copyright and license details. */
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define LEN(x) (sizeof(x) / sizeof(*(x)))
#define NR_ARGS	32

#undef dprintf
int dprintf(int, const char *, ...);
#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);
int strverscmp(const char *, const char *);
int spawnvp(char *, char *, char *[]);
int spawnlp(char *, char *, char *, ...);

#ifdef DEBUG
#define DEBUG_FD 8
#define DPRINTF_D(x) dprintf(DEBUG_FD, #x "=%d\n", x)
#define DPRINTF_U(x) dprintf(DEBUG_FD, #x "=%u\n", x)
#define DPRINTF_S(x) dprintf(DEBUG_FD, #x "=%s\n", x)
#define DPRINTF_P(x) dprintf(DEBUG_FD, #x "=0x%p\n", x)
#define DPRINTF_LLU(x) dprintf(DEBUG_FD, #x "=%llu\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#define DPRINTF_LLU(x)
#endif /* DEBUG */
