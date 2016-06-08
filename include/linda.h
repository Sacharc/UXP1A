
#ifndef TEST_LINDA_H
#define TEST_LINDA_H

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#define TUPLE_COUNT 128
#define TUPLE_CONTENT_LENGTH 128

struct tuple
{
    char tuple_content[TUPLE_CONTENT_LENGTH];
};

struct mem
{
    size_t tuple_count;
    struct tuple first_tuple[TUPLE_COUNT];
};

bool linda_init();
void linda_end();

size_t int_to_tuple(int input, char * output);
size_t double_to_tuple(double input, char * output);
size_t string_to_tuple(const char * input, char * output);

bool linda_output(const char * info_string, ...);
bool vlinda_output(const char * info_string, va_list * v_init);

bool compare_string(const char * operator_, const char * string_a, const char * string_b);
bool compare_int(const char * operator_, int a, int b);
bool compare_double(const char * operator_, double a, double b);

bool info_string_match_string_equals(const char * info_string, const char * match_string);
const char * match_string_extract_operator(const char * match_string, char * output_operator);
bool tuple_match_match_string(const struct tuple * tuple_to_match, const char * match_string);

int extract_tuple_from_shmem(const char * match_string);

bool linda_input(int timeout, const char * match_string, ...);
bool vlinda_input(int timeout, const char * match_string, va_list * v_init);

bool linda_read(int timeout, const char * match_string, ...);
bool vlinda_read(int timeout, const char * match_string, va_list * v_init);


#endif //TEST_LINDA_H