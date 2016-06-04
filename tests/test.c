#include <stdio.h>
#include <unistd.h>
#include "../include/linda.h"

int main() {
    linda_init();
    sleep(5);
    linda_end();
    return 0;
}