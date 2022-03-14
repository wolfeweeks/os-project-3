#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  sleep(10);
  printf("slave #%s\n", argv[1]);
}