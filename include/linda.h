
#ifndef TEST_LINDA_H
#define TEST_LINDA_H
#define TUPLE_SIZE	128

int linda_output(char * infoString, ...);
char* linda_input(int timeout, char* krotka);
char* linda_read(int timeout, char* krotka);
int linda_init();
int linda_end();

#endif //TEST_LINDA_H
