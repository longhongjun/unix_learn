#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
  #ifdef DEBUG
    printf("Compiled: "__DATE__"at "__TIME__"\n");
    printf("This is line %d of file %s\n", __LINE__, __FILE__);
  #endif
    printf("hello world\n");
  return 0;
}
