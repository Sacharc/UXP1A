
#ifndef TEST_LINDA_H
#define TEST_LINDA_H

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#define TUPLE_SIZE	128

int linda_init();
int linda_end(int segment_id);

//zwaraca czy udało się wstawić krotkę
bool linda_output(char * infoString, ...);
bool vlinda_output(char * infoString, va_list list);
//zwraca czy znaleziono krotkę zgodną z podanym wzorcem lub timeout
bool linda_input(int timeout, char* match_string, ...);
bool vlinda_input(int timeout, char* match_string, va_list list);
//zwraca czy znaleziono krotkę zgodną z podanym wzorcem lub timeout
bool linda_read(int timeout, char* match_string, ...);
bool vlinda_read(int timeout, char* match_string, va_list list);

#endif //TEST_LINDA_H