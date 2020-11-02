/*
  Utilities.c
  Project

  Created by Vittorio Cortosi on 13/01/2020.
  Copyright © 2020 Vittorio Cortosi. All rights reserved.
*/

#include "Utilities.h"

int rm_old_ipc(char* buf){
    char key[10];
    int i = 0;
    char* aux = buf;
    for(buf++; *buf != '\0'; buf++, i++){
        key[i] = *buf;
    }
    key[i++] = '\0';
    switch(aux[0]){
        case 'S':
            semctl(atoi(key), 0, IPC_RMID);
            return 0;
        case 'H':
            shmctl(atoi(key), IPC_RMID, 0);
            return 0;
    }
    return -1;
}

void check_old_ipcs(){
    printf(ANSI_COLOR_GREEN"LOOKING FOR OLD IPCS...\n"ANSI_COLOR_RESET);
    int fd = 0, i = -1, flag = 1;
    char buf;
    char* line = (char*)malloc(20 * sizeof(char));
    char* aux = line;
    if((fd = open(PATH_IPCS, O_RDONLY)) != -1){
        while((i = read(fd, &buf, 1) != 0)){
            flag = 0;
            if(buf == '\n'){
                *line++ = '\0';
                rm_old_ipc(aux);
                line = aux;
            }else{
                *line = buf;
                line++;
            }
        }
        if(flag){
            printf(ANSI_COLOR_GREEN"NO OLD IPCS FOUND..\n"ANSI_COLOR_RESET);
        }else{
            printf(ANSI_COLOR_RED"THERE WAS OLD IPCS.. REMOVED SUCCESSFULLY.\n"ANSI_COLOR_RESET);
        }
    }else{
        printf(ANSI_COLOR_GREEN"CANNOT FIND FILE WITH ODL IPCS\n"ANSI_COLOR_RESET);
    }
    free(aux);
    close(fd);
}

char* _itoa(const int val){
      char* str = malloc(10);
      bzero(str, 10);
      sprintf(str, "%d", val);
      return str;
}

void reset_ipc_fd(){
    int rfd = open(PATH_IPCS, O_CREAT | O_RDWR | O_TRUNC);
    close(rfd);
}

key_t get_ipcs(int type, int size, int permit){
    char buf[20] = {0};
    int key = -1, fd = 0;
    switch(type){
        case SEM:
            key = semget(IPC_PRIVATE, size, 0666);
            strcat(buf, "S");
            break;
        case SHM:
            key = shmget(IPC_PRIVATE, size, 0666);
            strcat(buf, "H");
            break;
    }
    strcat(buf, _itoa(key));
    if(key != -1){
        if((fd = open(PATH_IPCS, O_CREAT | O_RDWR | O_APPEND)) != -1){
            write(fd, buf, strlen(buf));
            write(fd, "\n", 1);
        }else{
            printf("Can't save IPC OBject on file descriptor..\nCLosing...\n");
            TEST_ERROR
            exit(EXIT_FAILURE);
        }
    }else{
        printf("Can't create the IPC Object: %d [%s]\nClosing..\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(fd);
    return key;
}
