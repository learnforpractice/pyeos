#include <stdio.h>
#include <world.h>

void printstr(char* s) {
   printf("%llx ------------- %s\n", s, s);
}

int printworld(int y) {
   // Convert Go string to C string (char*) and back.
  printf("World\n");
  return y;
}
