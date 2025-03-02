#include <stdio.h>
#include <stdlib.h>

#include "printutil.h"

#define OUTSTREAM stdout

void printOUT(char * msg){
    fputs(msg, OUTSTREAM);
}

void printLineOUT(char * msg){
    printOUT(msg);
    printOUT("\n");
}

void printNotice(char * msg){
    printOUT("[NOTICE] ");
    printLineOUT(msg);
}

void printWarning(char * msg){
    printOUT("[WARNING] ");
    printLineOUT(msg);
}

void printAndExitFailure(char * msg){
    printOUT("[FAILURE] ");
    printLineOUT(msg);
    exit(EXIT_FAILURE);
}

void printFailure(char * msg){
    printOUT("[FAILURE] ");
    printLineOUT(msg);
}
