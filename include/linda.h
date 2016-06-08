
#ifndef TEST_LINDA_H
#define TEST_LINDA_H

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#define TUPLE_COUNT 128
#define TUPLE_CONTENT_LENGTH 128

/*
 * Struct of single tuple
 * Tuple content stores bytes representing strings, integers and doubles
 */
struct tuple
{
    char tuple_content[TUPLE_CONTENT_LENGTH];
};

/*
 * Struct of memory, which stores array of tuple structures and
 * counter of stored structures in memory
 */
struct mem
{
    size_t tuple_count;
    struct tuple first_tuple[TUPLE_COUNT];
};

/*
 * Init function for linda API
 * Allocating a shared memory segment for struct mem
 * Init mem->tuple_count = 0
 * Return true if success, false if error
 */
bool linda_init();

/*
 * End function for linda API
 * Detach the shared memory segment.
 * Deallocate the shared memory segment if it's last process.
*/
void linda_end();

/*
 * Helper functions for linda_output
 */
size_t int_to_tuple(int input, char * output);
size_t double_to_tuple(double input, char * output);
size_t string_to_tuple(const char * input, char * output);

/*
 * Creates single tuple which represents arguments in va_list
 * It stored it in struct mem (shared memory) created by linda_init()
 * Working similar to printf
 * Return true if success, false if error
 */
bool linda_output(const char * info_string, ...);
bool vlinda_output(const char * info_string, va_list * v_init);

/*
 * Helper functions for linda_input
 */
bool compare_string(const char * operator_, const char * string_a, const char * string_b);
bool compare_int(const char * operator_, int a, int b);
bool compare_double(const char * operator_, double a, double b);

bool info_string_match_string_equals(const char * info_string, const char * match_string);
const char * match_string_extract_operator(const char * match_string, char * output_operator);
bool tuple_match_match_string(const struct tuple * tuple_to_match, const char * match_string);

int extract_tuple_from_shmem(const char * match_string);

bool linda_in_generic(bool to_remove, int timeout, const char * match_string, ...);
bool vlinda_in_generic(bool to_remove, int timeout, const char * match_string, va_list * v_init);

/*
 * Search for tuple matched with match_string, and assign values for arguments in va_list
 * Found tuple is deleted after reading it
 * Working similar to scanf
 * Return true if success, false if error
 */
bool linda_input(int timeout, const char * match_string, ...);
bool vlinda_input(int timeout, const char * match_string, va_list * v_init);

/*
 * Search for tuple matched with match_string, and assign values for arguments in va_list
 * Found tuple is not deleted after reading it
 * Working similar to scanf
 * Return true if success, false if error
 */
bool linda_read(int timeout, const char * match_string, ...);
bool vlinda_read(int timeout, const char * match_string, va_list * v_init);


#endif //TEST_LINDA_H