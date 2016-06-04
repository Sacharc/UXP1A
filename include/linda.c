//
// Created by michto on 04.06.16.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linda.h"



//1
struct mem {
    char* tuples; //tablica charow bez poczatkowego inta
    unsigned int tupleCount;
};
struct mem* shmemory;

////2
//struct mem {
//    char* tuples[]; //tablica tablic charow - czyli krotek
//    unsigned int tupleCount = 0;
//}
//struct mem shmemory*;
//
////3
//char* shmemory; //wszystko w jedna tablice charow wraz z poczatkowym intem

void checkIfNotExceeded (int * position) {
    if(*position == TUPLE_SIZE){
        perror("Too much arguments");
        exit(0);
    }
}


void parseNumberToTuple (const void *data, char* tuple, int * position){
    char *parsedData;
    parsedData = (char *) malloc(sizeof &data);

    memcpy(parsedData, data, sizeof &data);

    for (int i = 0; i < sizeof &data; ++i) {
        checkIfNotExceeded(position);
        tuple[*position] = parsedData[i];
        ++(*position);
    }
}


void parseStringToTuple (const void *data, char* tuple, int * position){
    int i = 0;
    while(1) {
        checkIfNotExceeded(position);
        tuple[*position] = (char)&data[i];
        ++position;
        ++i;

        if(((char)&data[i]) == '\0')
            break;
    }
}


int linda_output(char * infoString, ...) {
    va_list vl;
    int i;
    char tuple[TUPLE_SIZE];
    int position = 0;

    //infoString to tuple
    while(1){
        checkIfNotExceeded(position);
        tuple[position] = infoString[position];
        ++position;

        if(infoString[position] == '\0')
            break;
    }

    //data to tuple
    va_start( vl, infoString);
    for( i = 0; infoString[i] != '\0'; ++i ) {

        union types {
            int     i;
            float   f;
            char   *s;
        } types;

        switch( infoString[i] ) {   // Type to expect.
            case 'i':
                types.i = va_arg( vl, int );
                parseNumberToTuple(&types.i, tuple, &position);
                break;

            case 'f':
                types.f = va_arg( vl, double );
                parseNumberToTuple(&types.i, tuple, &position);
                break;

            case 's':
                types.s = va_arg( vl, char * );
                parseStringToTuple(&types.i, tuple, &position);
                break;

            default:
                perror("Not recognized");
                break;
        }
    }
    va_end( vl );

    //sposob 1
    shmemory->tuples = (char *) realloc(shmemory->tuples, shmemory->tupleCount * TUPLE_SIZE);
    memcpy(shmemory->tuples + shmemory->tupleCount * TUPLE_SIZE, tuple, TUPLE_SIZE);
    shmemory->tupleCount++;

    return 0;
}
