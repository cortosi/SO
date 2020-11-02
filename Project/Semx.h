/*
  semx.h
  Project

  Created by Vittorio Cortosi on 15/01/2020.
  Copyright © 2020 Vittorio Cortosi. All rights reserved.
*/

#ifndef semx_h
#define semx_h

#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>

/*SEMAPHORES UTILITY FUNCTIONS*/
int set_sem (int sem_id, int index, int val);

void dec_sem (int sem_id, int index);

void rel_sem(int sem_id, int index);

void wait_sem_zero (int sem_id, int index);

void reset_sem_set (int sem_id, int dim);

#endif /* semx_h */
