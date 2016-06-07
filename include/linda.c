//
// Created by michto on 04.06.16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linda.h"
#include <stdarg.h>

#define true 1
#define false 0

typedef int bool;


//TODO gdzie pakowac krotki ????
struct mem {
    char* tuples; //tablica charow bez poczatkowego inta
    unsigned int tupleCount;
};
struct mem* shmemory;


void checkIfNotExceeded (size_t length) {
    if(length >= TUPLE_SIZE){
        perror("Too much arguments");
        exit(0);
    }
}


void parseNumberToTuple (const void *data, char* tuple, int * position, bool isInt){
    if (isInt == true){
        checkIfNotExceeded(*position + sizeof (int));
        memcpy(tuple + *position, data, sizeof (int));
        *position = *position + sizeof (int);
    }
    else {
        checkIfNotExceeded(*position + sizeof (float));
        memcpy(tuple + *position, data, sizeof (float));
        *position = *position + sizeof (float);
    }
}


void parseStringToTuple (const void *data, char* tuple, int * position){
    size_t size = strlen(data) + 1;
    checkIfNotExceeded(*position + strlen(data) + 1);
    memcpy(tuple + *position, data, strlen(data) + 1);
    *position = *position + strlen(data) + 1;
}


int linda_output(char * infoString, ...) {
    va_list vl;
    int i;
    char tuple[TUPLE_SIZE];
    int position = 0;

    //infoString to tuple
    checkIfNotExceeded(strlen(infoString) + 1);
    memcpy(tuple, infoString, strlen(infoString) + 1); // + NULL
    position = position + strlen(infoString) + 1;

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
                parseNumberToTuple(&types.i, tuple, &position, true);
                break;

            case 'f':
                types.f = va_arg( vl, double );
                parseNumberToTuple(&types.f, tuple, &position, false);
                break;

            case 's':
                types.s = va_arg( vl, char * );
                parseStringToTuple(types.s, tuple, &position);
                break;

            default:
                perror("Not recognized");
                break;
        }
    }
    va_end( vl );

    //sposob 1
//    shmemory->tuples = (char *) realloc(shmemory->tuples, shmemory->tupleCount * TUPLE_SIZE);
//    memcpy(shmemory->tuples + shmemory->tupleCount * TUPLE_SIZE, tuple, TUPLE_SIZE);
//    shmemory->tupleCount++;

    return 0;
}
