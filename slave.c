/**
 * @file slave.c
 * @author Wolfe Weeks
 * @date 2022-03-17
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include "union.h"
#include "config.h"

static FILE* file;
static FILE* logfile;

// reverse and itoa from: https://stackoverflow.com/questions/190229/where-is-the-itoa-function-in-linux
/* reverse:  reverse string s in place */
void reverse(char s[]) {
  int i, j;
  char c;

  for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[]) {
  int i, sign;

  if ((sign = n) < 0)  /* record sign */
    n = -n;          /* make n positive */
  i = 0;
  do {       /* generate digits in reverse order */
    s[i++] = n % 10 + '0';   /* get next digit */
  } while ((n /= 10) > 0);     /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}

/**
 * @brief wait until process' turn (using semaphores) to enter critical section
 *
 * @param semId semaphore id
 * @param sb semaphore buffer
 * @param procNo process number
 * @return ** void
 */
void lock(int semId, struct sembuf sb, int procNo) {
  int locked = 0; //boolean for whether or not the semaphore equals 0
  union semun arg;

  //get time and format it according to HH:MM:SS
  struct tm* timeInfo;
  time_t rawtime;
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  char formattedTime[9];
  strftime(formattedTime, 9, "%X", timeInfo);

  //write to respective logfile and close it
  fprintf(logfile, "%s Process %d attempting to enter critical section\n", formattedTime, procNo);
  fclose(logfile);

  sb.sem_op = -1;
  while (locked == 0) {
    //try to decrement semaphore
    if (semop(semId, &sb, 1) == -1) {
      perror("runsim: Error: semop");
      exit(1);
    } else {
      locked = 1;
    }
  }
  return;
}

/**
 * @brief reset the semaphore to 1 allowing next process to enter critical
 *        section
 *
 * @param semId semaphore id
 * @param sb semaphore buffer
 * @param procNo process number
 * @return ** void
 */
void unlock(int semId, struct sembuf sb, int procNo) {
  //get time and format it according to HH:MM:SS
  struct tm* timeInfo;
  time_t rawtime;
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  char formattedTime[9];
  strftime(formattedTime, 9, "%X", timeInfo);

  //write to logfile and close it
  fprintf(logfile, "%s Process %d leaving critical section\n", formattedTime, procNo);
  fclose(logfile);

  //update semaphore to 1 allowing other processes to enter critical section
  sb.sem_op = 1;
  if (semop(semId, &sb, 1) == -1) {
    perror("runsim: Error: semop");
    exit(1);
  } else {
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
  return random;
}

int main(int argc, char* argv[]) {
  srand(time(NULL)); //seed random number generator

  //get semaphore
  key_t key = ftok(MASTER, 1);
  int semId = semget(key, 1, IPC_CREAT);

  struct sembuf sb = { 0, -1, 0 }; //set semaphore operation to decrement

  int procNo = atoi(argv[1]) + 1; //set process number

  //get the file name for this process' logfile
  int sizeOfLogfileName = sizeof(char)
    * (strlen("logfile." + strlen(argv[1]) + 1));
  char* logfileName = (char*)malloc(sizeOfLogfileName);
  strcat(logfileName, "logfile.");
  char procNoString[3];
  itoa(procNo, procNoString);
  strcat(logfileName, procNoString);

  //create/clear logfile
  logfile = fopen(logfileName, "w");
  fclose(logfile);

  struct tm* timeInfo;
  time_t rawtime;

  int i;
  for (i = 0; i < 5; i++) {
    logfile = fopen(logfileName, "a"); //open logfile
    lock(semId, sb, procNo); //lock all other processes out of critical section

    //critical section**********************************************************
    sleep(randomSleepTime()); //sleep for [1,5] seconds

    //set the current time
    time(&rawtime);
    timeInfo = localtime(&rawtime);
    char formattedTime[9];
    strftime(formattedTime, 9, "%X", timeInfo);

    //open, write to, and close the logfile
    logfile = fopen(logfileName, "a");
    fprintf(logfile, "%s File modified by process number %d\n",
      formattedTime, procNo);
    fclose(logfile);

    //open the critical file, write to it, and close it
    file = fopen("cstest", "a");
    fprintf(file, "%s File modified by process number %d\n",
      formattedTime, procNo);
    fclose(file);

    sleep(randomSleepTime()); //sleep for [1,5] seconds
    //critical section**********************************************************

    logfile = fopen(logfileName, "a"); //open logfile
    unlock(semId, sb, procNo); //unlock semaphore
  }

  return 0;
}