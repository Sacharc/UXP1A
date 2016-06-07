
#ifndef TEST_LINDA_H
#define TEST_LINDA_H

#include <stdbool.h>

#define TUPLE_SIZE	128

//zwaraca czy udało się wstawić krotkę
bool linda_output(char * infoString, ...);
//zwraca czy znaleziono krotkę zgodną z podanym wzorcem lub timeout
bool linda_input(int timeout, char* match_string, ...);
//zwraca czy znaleziono krotkę zgodną z podanym wzorcem lub timeout
bool linda_read(int timeout, char* match_string, ...);
int linda_init();
int linda_end();

#endif //TEST_LINDA_H