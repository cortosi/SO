/*
  Utilities.h
  SOProject

  Created by Vittorio Cortosi on 16/12/2019.
  Copyright © 2019 Vittorio Cortosi. All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Handling.h"

#ifndef Utilities_h
#define Utilities_h

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifndef SO_NUM_G
#define SO_NUM_G 2
#endif
#ifndef SO_NUM_P
#define SO_NUM_P 10
#endif
#ifndef SO_MAX_TIME
#define SO_MAX_TIME 3
#endif
#ifndef SO_BASE
#define SO_BASE 60
#endif
#ifndef SO_ALTEZZA
#define SO_ALTEZZA 20
#endif
#ifndef SO_FLAG_MIN
#define SO_FLAG_MIN 5
#endif
#ifndef SO_FLAG_MAX
#define SO_FLAG_MAX 5
#endif
#ifndef SO_ROUND_SCORE
#define SO_ROUND_SCORE 10
#endif
#ifndef SO_N_MOVES
#define SO_N_MOVES 20
#endif
#ifndef SO_MIN_HOLD_NSEC
#define SO_MIN_HOLD_NSEC 100000000
#endif
#define FLAG_CH 64
#define SEM 0
#define SHM 1
#define SMM 2
#define PATH_IPCS "/tmp/old_ipcs.txt"

#define SLEEP1 sleep(1);
#define SLEEP2 sleep(2);
#define SLEEP3 sleep(3);

#define RESET_ALRM alarm(0);
#define ALARM alarm(SO_MAX_TIME);

struct chessboard{
    char mat[SO_ALTEZZA][SO_BASE];
};
/*---------------------------------------------*/

struct pl_det{
    pid_t pid;
    char sym;
    int score;
};

struct flg_det{
    pid_t owner;
    int alt;
    int base;
    int dist;
};

static int cont_player = 0;

/*MACRO USED TO TEST ERRNO*/
#define TEST_ERROR  if (errno) {\
                    fprintf(stderr,\
                    "Errno Value: %d, meaning: %s\n",\
                    errno,\
                    strerror(errno));\
                    } else\
                    printf("Non ci sono errori a quanto pare.. errno: %d\n", errno);

#define PRINT_STATUS    for(_a = 0; _a < SO_ALTEZZA; _a++){\
                            printf("\n");\
                            if(_a < 10) printf("%d ", (_a));\
                            else printf("%d", (_a));\
                            for(_b = 0; _b < SO_BASE; _b++){\
                                if(cb -> mat[_a][_b] != '\0'){\
                                    if(cb -> mat[_a][_b] == (char)FLAG_CH){\
                                        printf(ANSI_COLOR_RED"|%c|"ANSI_COLOR_RESET, cb -> mat[_a][_b]);\
                                    }else{\
                                        printf("|%c|", cb -> mat[_a][_b]);\
                                    }\
                                } else {\
                                        printf("| |");\
                                }\
                            }\
                        }\
                        printf("\n");

#define PRINT_SCORES    printf(ANSI_COLOR_CYAN"PID\tPLAYER\tSCORE\n");\
                        for(i = 0; i < SO_NUM_G; i++){\
                            printf(ANSI_COLOR_RESET"%d\t%c\t%d\n", players[i].pid, players[i].sym, players[i].score);\
                        }\
                        printf("\n");

#define SEMLIST system("ipcs -s");
#define SHMLIST system("ipcs -m");
#define IPCLIST system("ipcs");

void reset_ipc_fd(void);

int rm_old_ipc(char* buf);

void check_old_ipcs(void);

char* _itoa(const int val);

key_t get_ipcs(int type, int size, int permit);



#endif /* Utilities_h */
