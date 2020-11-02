#define _POSIX_C_SOURCE 199309L
#include "Utilities.h"
/*FUNCTIONS PROTOTYPE*/
int check_sym(struct pl_det* sm, char sym);

struct pl_det* addpl(struct pl_det* players);

int init_flag(pid_t owner, int x, int y);

int moveup(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves);

int movedw(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves);

int movelt(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves);

int movert(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves);

/*MAIN START*/
int main(int argc, char* argv[]){
    /*CLEARING OLD IPCS, IF PRESENT*/
    check_old_ipcs();
    reset_ipc_fd();

    /*VARIABLES*/
    struct timespec my_time;
    my_time.tv_sec = 0;
    my_time.tv_nsec = SO_MIN_HOLD_NSEC;
    pid_t childs, p_pid, pa_pid;
    key_t sh_mem_cb, sh_mem_arr_p, sh_sem_sync, sh_sem_cb, sh_mem_flgs, sh_mem_answ;
    int i = 0, j = 0, status, x, y, _a, _b;
    char pl_ch, *answ;
    struct pl_det* pl;
    srand(getpid());

    /*SHARED VARIABLES POINTERS*/
    struct chessboard* cb;  /*CHESSBOARD*/
    struct pl_det* players; /*PLAYERS DETAIL*/
    struct flg_det* flags;  /*FLAGS DETAIL*/

    static int nround = 0;

    /*SETTING SIGNAL HANDLER*/
    struct sigaction sa;
    bzero(&sa, sizeof(struct sigaction));
    sa.sa_handler = signal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    /*START MASTER CODE*/
    printf(ANSI_COLOR_YELLOW"(MASTER), PID: %d\n"ANSI_COLOR_RESET, getpid());
    SLEEP1
    /*IPC OBJECTS CREATION/ALLOCATION*/
    sh_sem_sync = get_ipcs (SEM, 8, 0666);    /*SEMSET FOR SYNCRONIZATIONS*/
    sh_mem_cb = get_ipcs (SHM, sizeof(struct chessboard), 0666); /*SHARED MEMORY FOR CHESSBOARD*/
    sh_mem_arr_p = get_ipcs (SHM, (sizeof(struct pl_det)) * SO_NUM_G, 0666); /*SHARED MEMORY CONTANING PLAYERS INFOS*/
    sh_sem_cb = get_ipcs (SEM, (SO_BASE * SO_ALTEZZA) , 0666);    /*SEM FOR THE ALL CHESSBOARD*/
    sh_mem_flgs = get_ipcs (SHM, (sizeof(struct flg_det)) * SO_FLAG_MAX, 0666); /*SHARED MEMORY CONTANING FLAGS DETAILS*/
    sh_mem_answ = get_ipcs (SHM, sizeof(char), 0666);   /*SHARED MEMORY FOR THE ROUND ANSWARE*/
    
    /*IPC OBJECT ATTACHING*/
    cb = shmat(sh_mem_cb, NULL, 0);
    players = shmat(sh_mem_arr_p, NULL, 0);
    flags = shmat(sh_mem_flgs, NULL, 0);
    answ = shmat(sh_mem_answ, NULL, 0);
    reset_sem_set(sh_sem_sync, 8);
    
    /*CHILD PROCESS CREATIONS*/
    for(i = 0; i < SO_NUM_G; i++){
        switch(p_pid = fork()){
            case -1:{
                fprintf(stderr,ANSI_COLOR_YELLOW"(MASTER) PID: %d, CANNOT CREATE CHILD PROCESS FOR PLAYERS.. SOMETHING WENT WRONG.\n"ANSI_COLOR_RESET, getpid());
                TEST_ERROR
                fprintf(stderr, "(MASTER) EXITING..\n");
                SLEEP2
                exit(EXIT_FAILURE);
            }
            case 0:{
                /*PLAYER OPS*/
                srand(getpid());
                pl = addpl(players);
                pl->pid = getpid();
                /*CHOSING A RANDOM SYMBOL FOR CHESSBOARD*/
                do{
                    pl_ch = (rand() % (90 - 65 + 1)) + 65;
                }while(check_sym(players, (char)pl_ch) != 0);
                pl->sym = pl_ch;
                printf("(PLAYER) PID: %d, SYMBOL: %c\n", pl->pid, pl->sym);
                SLEEP2
                /*CREATING PAWNS*/
                do{
                    wait_sem_zero(sh_sem_sync, 7); /*WAITING MASTER RESET SEMSETS AND SHMEMS*/
                    for(j = 0; j < SO_NUM_P; j++){
                        switch(pa_pid = fork()){
                            case -1:{
                                fprintf(stderr, ANSI_COLOR_RED"(PLAYER) PID: %d, CANNOT CREATE PROCESS FOR PAWNS, SOMWTHING WENT WRONG.\n",
                                    getpid());
                                TEST_ERROR
                                fprintf(stderr, "EXITING..");
                                SLEEP2
                                exit(EXIT_FAILURE);
                                break;
                            }
                            case 0:{
                                /*PAWNS EXECUTING*/
                                srand(getpid());
                                int base, altezza, n = sizeof(int), i;
                                int* indexs, *aux;
                                /*PLACING PAWNS RANDOMLY*/
                                do{
                                    altezza = (rand() % (20  + 1));
                                    base = (rand() % (60  + 1));
                                }while (semctl(sh_sem_cb, ((altezza * 60) + base), GETVAL) != 1);
                                dec_sem(sh_sem_cb, (altezza * 60) + base); /*FILL SEMAPHORE*/
                                cb-> mat[altezza][base] = (char)pl->sym;
                                dec_sem(sh_sem_sync, 0); /*INFORM THAT PAWN HAS BEEN PLACED*/
                                wait_sem_zero(sh_sem_sync, 1); /*WAITING FOR MASTER FLAGS PLACING*/
                                dec_sem(sh_sem_sync, 3); /*SYNC BETWEEN PAWNS (EXECUTE ONE BY ONE)*/
                                indexs = malloc(sizeof(int) * (SO_FLAG_MAX + 1));
                                memset (indexs, -1, sizeof(int) * (SO_FLAG_MAX + 1));
                                indexs[SO_FLAG_MAX] = -2;

                                printf("(PAWN) PID: %d, PLACED AT [%d][%d]\n", getpid(), altezza, base);
                                /*CALCUILATING FLAGS DISTANCE*/
                                aux = indexs;
                                for(i = 0; i < SO_FLAG_MAX; i++){
                                    n = (abs(altezza - flags[i].alt) + (abs(base - flags[i].base)));
                                    if((flags[i].owner == -1) || (flags[i].owner != -1 && flags[i].dist > n)){
                                        flags[i].owner = getpid();
                                        flags[i].dist = n;
                                        *indexs = i;
                                        indexs++;
                                    }
                                }
                                /*END CLALCULATING*/
                                rel_sem(sh_sem_sync, 3);    /*SYNC BETWEEN PAWNS (EXECUTE ONE BY ONE)*/
                                dec_sem(sh_sem_sync, 4);    /*INFORM THAT PAWN HAS COMPLETED HIS CALCULATION*/
                                wait_sem_zero(sh_sem_sync, 4); /*WAIT FOR ALL PAWNS CALCULATED FLAGS DISTANCES*/
                                /*CHECKING FLAGS*/
                                for(indexs = aux; *indexs != -2; indexs++){
                                    if((*indexs != -1) && flags[*indexs].owner == getpid()){
                                        /*PAWN IS ALREADY MINE*/
                                    }else{
                                        *indexs = -1; /*PAWN REPLACED BY SOMEONELSE, NOT MINE*/
                                    }
                                }
                                
                                /*PAWN START MOVING*/
                                int delta = -1, status = 1, moves = SO_N_MOVES;
                                dec_sem(sh_sem_sync, 3);
                                indexs = aux;
                                for(indexs = aux; *indexs != -2; indexs++){
                                    if(*indexs != -1){
                                        for(; ((status) && (flags[*indexs].alt != altezza || flags[*indexs].base != base)); ){
                                            if((delta = flags[*indexs].base - base) < 0){
                                                for(;status && delta < 0 && moves > 0; delta++){
                                                    status = movelt(&altezza, &base, cb, sh_sem_cb, pl->sym, &moves);
                                                }
                                            }else if((delta = flags[*indexs].base - base) > 0){
                                                for(;status && delta > 0 && moves > 0; delta--){
                                                    status = movert(&altezza, &base, cb, sh_sem_cb, pl->sym, &moves);
                                                }
                                            }else if((delta = flags[*indexs].alt - altezza) > 0){
                                                for(;status && delta > 0 && moves > 0; delta--){
                                                    status = movedw(&altezza, &base, cb, sh_sem_cb, pl->sym, &moves);
                                                }
                                            }else if((delta = flags[*indexs].alt - altezza) < 0){
                                                for(;status && delta < 0 && moves > 0; delta++){
                                                    status = moveup(&altezza, &base, cb, sh_sem_cb, pl->sym, &moves);
                                                }
                                            }
                                            nanosleep(&my_time, NULL);
                                        }
                                        if(status){
                                            printf(ANSI_COLOR_CYAN"(PAWN) PID: %d, FLAG [%d][%d] CAPTURED, %d POINTS GAINED\n"ANSI_COLOR_RESET,getpid(), flags[*indexs].alt, flags[*indexs].base, SO_ROUND_SCORE/SO_FLAG_MAX);
                                            pl->score += SO_ROUND_SCORE/SO_FLAG_MAX;
                                        }else{
                                            printf("(PAWN) PID: %d, STUCKED AT [%d][%d], AN OBSTACLE OCCURED\n", getpid(), altezza, base);
                                        }
                                    }
                                }
                                rel_sem(sh_sem_sync, 3);
                                free(aux);
                                exit(EXIT_SUCCESS);
                                break;
                            }
                        }
                    }
                    /*WAITING FOR PAWNS ENDING*/
                    while((childs = wait(&status)) != -1){
                        /*printf("(PLAYER) PID: %d, PAWN ENDED ITS EXECUTION: %d, ITS STATUS: %d\n", childs, status);*/
                        /*TEST_ERROR*/
                    }
                    dec_sem(sh_sem_sync, 5);
                    alarm(0);
                    wait_sem_zero(sh_sem_sync, 6);
                }while(*answ == 'Y');
                exit(EXIT_SUCCESS);
            }
        }
    }
    do{
        bzero(cb, sizeof(struct chessboard));
        reset_sem_set(sh_sem_sync, 8);
        semctl(sh_sem_sync, 0, SETVAL, SO_NUM_P * SO_NUM_G);
        semctl(sh_sem_sync, 4, SETVAL, SO_NUM_P * SO_NUM_G);
        semctl(sh_sem_sync, 5, SETVAL, SO_NUM_G);
        reset_sem_set(sh_sem_cb, (SO_BASE * SO_ALTEZZA));
        dec_sem(sh_sem_sync, 7);

        /*WAITING FOR PLAYERS' PAWNS PLACING*/
        printf(ANSI_COLOR_YELLOW"(MASTER) WAITING FOR PAWNS PLACING...\n"ANSI_COLOR_RESET);
        SLEEP1
        wait_sem_zero(sh_sem_sync, 0);  /*WAITING THAT ALL PAWNS ACK*/
        semctl(sh_sem_sync, 7, SETVAL, 1);

        /*PLACING FLAGS*/
        for(i = 0; i < SO_FLAG_MAX; i++){
            do{
                x = (rand() % (20  + 1));
                y = (rand() % (60  + 1));
            }while (semctl(sh_sem_cb, ((x * 60) + y), GETVAL) != 1);
            cb -> mat[x][y] = (char)FLAG_CH;
            flags[i].owner = -1;
            flags[i].alt = x;
            flags[i].base = y;
            flags[i].dist = -1;
        }
        printf(ANSI_COLOR_YELLOW"(MASTER) PAWNS PLACED... PLACING FLAGS, FLAG SYM: "ANSI_COLOR_RED);
        printf("%c\n"ANSI_COLOR_RESET, FLAG_CH);
        SLEEP3
        PRINT_STATUS
        printf("ROUND STARTED!!!\n");
        nround++;
        SLEEP2
        dec_sem(sh_sem_sync, 1);        /*INFORM THAT FLAGS HAVE BEEN PLACED*/
        wait_sem_zero(sh_sem_sync, 4);  /*WAIT FOR ALL PAWNS CALCULATED FLAGS DISTANCES TO START ROUNDS*/
        
        /*STARTING ROUND*/
        ALARM
        wait_sem_zero(sh_sem_sync, 5);
        RESET_ALRM
        PRINT_STATUS
        PRINT_SCORES
        printf("ROUND ENDED!!!\nWANNA PLAY AGAIN?? (Y/N):");
        scanf(" %c", answ);
        if(*answ == 'Y'){
            *answ = 'Y';
            printf("LET'S PLAY ANOHER ROUND!!\n");
        }else{
            *answ = 'N';
            printf(ANSI_COLOR_CYAN"ROUNDS PLAYED: %d\n//////END GAME//////\n"ANSI_COLOR_RESET, nround);
        }
        reset_sem_set(sh_sem_sync, 8);
        dec_sem(sh_sem_sync, 6);
    }while(*answ == 'Y');
    
    /*WAITING FOR PLAYERS ENDING*/
    while((childs = wait(&status)) != -1){
        printf(ANSI_COLOR_YELLOW"(MASTER) PID: %d, PLAYED EXITED (PID): %d, STATUS: %d\n"ANSI_COLOR_RESET, getpid(), childs, status);
        /*TEST_ERROR*/
    }
    
    /*DEATACH AND DELETE IPC OBJECT*/
    printf("REMOVING IPCS OPENED...\n");
    semctl(sh_sem_sync, 0, IPC_RMID);
    shmctl(sh_mem_arr_p, IPC_RMID, 0);
    shmctl(sh_mem_cb, IPC_RMID, 0);
    semctl(sh_sem_cb, 0, IPC_RMID);
    shmctl(sh_mem_flgs, IPC_RMID, 0);
    shmctl(sh_mem_answ, IPC_RMID, 0);
    
    reset_ipc_fd();
    exit(EXIT_SUCCESS);
}

struct pl_det* addpl(struct pl_det* players){
    int i = 0;
    for(; i < SO_NUM_G; i++, players++){
        if(players->pid == 0){
            return players;
        }
    }
    return NULL;
}

int check_sym(struct pl_det* sm, char sym){
    int z = 0;
    for(z = 0; z < SO_NUM_G; z++){
        if(sm -> sym == sym){
            return -1;
        }
    }
    return 0;
}

int movelt(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves){
    if((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 0) || ((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 1) && (cb -> mat[(*altezza) + 1][*base] = FLAG_CH))){
        cb -> mat[*altezza][*base] = '\0';
        rel_sem(sem, ((*altezza * 60) + *base));
        dec_sem(sem, ((*altezza * 60) + --(*base)));
        cb -> mat[*altezza][*base] = ch;
        moves--;
        return 1;
    }else{
        return 0;
    }
}

int movedw(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves){
    if((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 0) || ((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 1) && (cb -> mat[(*altezza) + 1][*base] = FLAG_CH))){
        cb -> mat[*altezza][*base] = '\0';
        rel_sem(sem, ((*altezza * 60) + *base));
        dec_sem(sem, (++(*altezza) * 60) + *base);
        cb -> mat[*altezza][*base] = ch;
        moves--;
        return 1;
    }else{
        return 0;
    }
}

int moveup(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves){
    if((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 0) || ((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 1) && (cb -> mat[(*altezza) + 1][*base] = FLAG_CH))){
        cb -> mat[*altezza][*base] = '\0';
        rel_sem(sem, ((*altezza * 60) + *base));
        dec_sem(sem, ((--(*altezza) * 60) + *base));
        cb -> mat[*altezza][*base] = ch;
        moves--;
        return 1;
    }else{
        return 0;
    }
}

int movert(int* altezza, int* base, struct chessboard* cb, key_t sem, char ch, int* moves){
    if((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 0) || ((semctl(sem, ((*altezza * 60) + *base), GETVAL) == 1) && (cb -> mat[(*altezza) + 1][*base] = FLAG_CH))){
        cb -> mat[*altezza][*base] = '\0';
        rel_sem(sem, ((*altezza * 60) + *base));
        dec_sem(sem, ((*altezza * 60) + ++(*base)));
        cb -> mat[*altezza][*base] = ch;
        moves--;
        return 1;
    }else{
        return 0;
    }
}