#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()

#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  static bool init = true;
  static char* malloc_addr;
  if(init)
  {
    malloc_addr = (void *)ROUNDUP(heap.start, 8);
    init = false;
  }
  size  = (size_t)ROUNDUP(size, 8);
  char *old = malloc_addr;
  malloc_addr += size;
  assert((uint32_t *)malloc_addr >= (uint32_t *)heap.start && (uint32_t *)malloc_addr <= (uint32_t *)heap.end);
  for (uint32_t *p = (uint32_t *)old; p != (uint32_t *)malloc_addr; p ++) {
    *p = 0;
  }
  return old;
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
