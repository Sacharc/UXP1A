
#ifndef TEST_TESTS_CONSTANTS_H
#define TEST_TESTS_CONSTANTS_H

#include <unistd.h>
#include <bits/time.h>

static const struct timeval t = {0, 1000};

extern "C" bool linda_init();
extern "C" void linda_end();
extern "C" bool linda_output(const char * info_string, ...);
extern "C" bool linda_input(struct timeval timeout, const char * match_string, ...);
extern "C" bool linda_read(struct timeval timeout, const char * match_string, ...);
extern "C" bool info_string_match_string_equals(const char * info_string, const char * match_string);

#endif //TEST_TESTS_CONSTANTS_H
