#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include "union.h"
#include "config.h"

void lock(int semId, struct sembuf sb) {
  int locked = 0;
  union semun arg;
  while (locked == 0) {
    if (semop(semId, &sb, 1) == -1) {
      perror("semop");
      exit(1);
    } else {
      printf("locking...\n");
      locked = 1;
    }
  }
  return;
}

void unlock(int semId, struct sembuf sb) {
  sb.sem_op = 1;
  if (semop(semId, &sb, 1) == -1) {
    perror("semop2");
    exit(1);
  } else {
    printf("unlocking...\n");
    return;
  }
}

/**
 * @brief returns a random number between 0 and 5 (inclusive)
 *
 * @return ** int
 */
int randomSleepTime() {
  int random = rand() % 6;
  // printf("\tSleeping for %d seconds\n", random);
  return random;
}

int main(int argc, char* argv[]) {
  srand(time(NULL));
  key_t key = ftok(MASTER, 1);
  int semId = semget(key, 1, IPC_CREAT);
  struct sembuf sb = { 0, -1, 0 };

  int i;
  for (i = 0; i < 5; i++) {
    lock(semId, sb);

    //critical section**********************************************************
    sleep(randomSleepTime());
    printf("%d\n", atoi(argv[1]) + 1);
    sleep(randomSleepTime());
    //critical section**********************************************************

    unlock(semId, sb);
  }

  return 0;
}