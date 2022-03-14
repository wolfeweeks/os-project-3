#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>

int* attachMem(char* fileName, int size);

bool detachMem(int* memory);

bool destroyMem(char* fileName);

#define MEM_SIZE 4096
#define CHOOSING "./master"
#define TICKET "./slave"
#define CSTEST "./cstest"

#endif // !SHARED_MEMORY_H