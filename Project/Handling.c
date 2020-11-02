/*
  Handling.c
  Project

  Created by Vittorio Cortosi on 15/01/2020.
  Copyright © 2020 Vittorio Cortosi. All rights reserved.
*/

#include "Handling.h"

void signal_handler(int signum){
    switch(signum){
        case SIGINT:
            printf("SIGINT received.. can't handle it.\n");
            break;
        case SIGALRM:
            printf("ROUND HAS LASTED TOO.\n");
            /*PRINT_METRIC*/
            printf("Exiting...\n");
            kill(0, SIGTERM);
            exit(EXIT_SUCCESS);
            break;
    }
}
