/**
 * @file master.c
 * @author Wolfe Weeks
 * @date 2022-03-17
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> //for getopt variables (e.g. opterr, optopt, etc.)
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include "config.h"
#include "union.h"

static int semId;
static union semun arg;

static FILE* logfile;

void anakin(int sig) {
  printf("Killing all child processes...\n");
  semctl(semId, 0, IPC_RMID, arg); //remove the semaphore
  signal(SIGQUIT, SIG_IGN);

  //set current time
  struct tm* timeInfo;
  time_t rawtime;
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  char formattedTime[9];
  strftime(formattedTime, 9, "%X", timeInfo);

  //open, write to, and close the logfile
  logfile = fopen("logfile.master", "w");
  fprintf(logfile, "%s All processes terminated\n", formattedTime);
  fclose(logfile);

  kill(0, SIGQUIT); //close all programs
}

int main(int argc, char* argv[]) {
  //set up signals to kill program
  signal(SIGALRM, anakin);
  signal(SIGINT, anakin);

  int numOfProcs;
  int maxTime = 100;

  union semun semUnion;

  // read through the user's command line options and set any necessary args
  int options = 0;
  while (options != -1) {
    options = getopt(argc, argv, "ht:");
    switch (options) {
    case 'h':
      printf("usage: master [-t maxSeconds] numOfProcs\n");
      return 0;
    case 't':
      maxTime = atoi(optarg);
      break;
    case ':':
      printf("Option needs a value\n");
      return 1;
    case '?':
      printf("%c is not an option\n", optopt);
      return 1;
    default:
      break;
    }
  }

  if (argc < 2 || argc > 4) // ensure proper number of args
  {
    fprintf(stderr, "usage: master [-t maxSeconds] numOfProcs\n");
    exit(1);
  }

  // set number of processes to the user entered amount or a max of 18
  if (argc == 2) {
    if (atoi(argv[1]) > MAX_CHILDREN) {
      printf("WARNING: Setting the number of child processes to 18\n");
      numOfProcs = MAX_CHILDREN;
    } else {
      numOfProcs = atoi(argv[1]);
    }
  } else {
    if (atoi(argv[3]) > MAX_CHILDREN) {
      printf("WARNING: Setting the number of child processes to 18\n");
      numOfProcs = MAX_CHILDREN;
    } else {
      numOfProcs = atoi(argv[3]);
    }
  }

  //create/clear cstest file and close it
  FILE* file = NULL;
  file = fopen("cstest", "w");
  fclose(file);

  key_t key = ftok(MASTER, 1); //get a key for the semaphore

  /* create a semaphore set with 1 semaphore: */
  if ((semId = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR)) == -1) {
    perror("runsim: Error: semget");
    exit(1);
  }
  /* initialize semaphore #0 to 1: */
  arg.val = 1;
  if (semctl(semId, 0, SETVAL, arg) == -1) {
    perror("runsim: Error: semctl");
    semctl(semId, 0, IPC_RMID, arg);
    exit(1);
  }

  pid_t childPid = 0;
  int status = 0;

  int i;
  for (i = 0; i < numOfProcs; i++) { //execute the specified number of child processes
    if (childPid = fork() == 0) { //if child process...
      char processNo[3];
      sprintf(processNo, "%d", i);
      char numOfProcsString[3];
      sprintf(numOfProcsString, "%d", numOfProcs);

      execl("./slave", "./slave", processNo, numOfProcsString, NULL);
    } else if (childPid == -1) {
      perror("runsim: Error: could not create child process");
      break;
    }
  }

  alarm(maxTime); //set a timeout for all processes

  while (wait(&status) > 0); //wait for all child processes to complete

  semctl(semId, 0, IPC_RMID, arg); //remove the semaphore

  //set current time
  struct tm* timeInfo;
  time_t rawtime;
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  char formattedTime[9];
  strftime(formattedTime, 9, "%X", timeInfo);

  //open, write to, and close the logfile
  logfile = fopen("logfile.master", "w");
  fprintf(logfile, "%s All processes terminated\n", formattedTime);
  fclose(logfile);

  return 0;
}
