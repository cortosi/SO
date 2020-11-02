/*
  semx.c
  Project

  Created by Vittorio Cortosi on 15/01/2020.
  Copyright © 2020 Vittorio Cortosi. All rights reserved.
*/

#include "Semx.h"

int set_sem(int sem_id, int index, int val)
{
    return semctl(sem_id, index, SETVAL, val);
}

void dec_sem (int sem_id, int index)
{
    struct sembuf sem_op;
    sem_op.sem_num  = index;
    sem_op.sem_op   = -1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

void rel_sem(int sem_id, int index)
{
    struct sembuf sem_op;
    sem_op.sem_num  = index;
    sem_op.sem_op   = 1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

void wait_sem_zero (int sem_id, int index)
{
    struct sembuf sem_op;
    sem_op.sem_num  = index;
    sem_op.sem_op   = 0;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

void reset_sem_set (int sem_id, int dim){
    int i = 0;
    for(i = 0; i < dim; i++){
        semctl(sem_id, i, SETVAL, 1);
    }
}
