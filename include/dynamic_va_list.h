// http://stackoverflow.com/a/30632712/5459240
#ifndef TEST_DYNAMIC_VA_LIST_H
#define TEST_DYNAMIC_VA_LIST_H

#include <stdio.h>
#include <stdlib.h>

#define VLIST_CHUNK_SIZE 8

typedef struct  {
    va_list _va_list;
    void* _va_list_ptr;
} dynamic_va_list;

void dynamic_va_start(dynamic_va_list* args, void* arg_list)
{
    args->_va_list[0].gp_offset = 48;
    args->_va_list[0].fp_offset = 304;
    args->_va_list[0].reg_save_area = NULL;
    args->_va_list[0].overflow_arg_area = arg_list;
    args->_va_list_ptr = arg_list;
}

void dynamic_va_end(dynamic_va_list* args)
{
    free(args->_va_list_ptr);
}

#endif //TEST_DYNAMIC_VA_LIST_H
