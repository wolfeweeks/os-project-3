union semun {
  int val;                /* value for SETVAL */
  struct semid_ds* buf;   /* buffer for IPC_STAT & IPC_SET */
  __u_short array;  /* array for GETALL & SETALL */
};