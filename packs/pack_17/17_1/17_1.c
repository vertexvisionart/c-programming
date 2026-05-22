#include <stdio.h>

int main(void) {

#if defined(__clang__)
  printf("Compiler: clang\n");
#elif defined(__TINYC__)
  printf("Compiler: TCC\n");
#elif defined(_MSC_VER)
  printf("Compiler: MSVC\n");
#elif defined(__GNUC__)
  printf("Compiler: GCC\n");
#else
  printf("Compiler: Unknown\n");
#endif

  printf("Bitness: %d\n", (int)(sizeof(void *) * 8));

#ifdef NDEBUG
  printf("Asserts: disabled\n");
#else
  printf("Asserts: enabled\n");
#endif

  return 0;
}
