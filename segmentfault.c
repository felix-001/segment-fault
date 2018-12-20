// Last Update:2018-12-20 14:24:11
/**
 * @file seg-fault-dump-registers.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-12-19
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define LOGI(args...) printf("[ % %s %d ] ", __FILE__, __FUNCTION__, __LINE__);printf(args)
#define ITEM( func ) { #func, func }

typedef struct {
    const char *str;
    const void *addr;
} func_t;

char *get_addr( char **p )
{
    char buffer[10];
    
    *p = buffer;
        
    return NULL;
}

void stack_overflow_crash()
{
    int i = 0;
    char *p1 = NULL;

    get_addr( &p1 );
    for ( i = 0; i<100000; i++ ) {
        p1[i] = 10;
    }
}

void write_zero_address_crash()
{
    char *p = NULL;

    *p = 12;
}

void crash_entry()
{
    write_zero_address_crash();
    stack_overflow_crash();
}


static func_t func_list[] = 
{
    ITEM( crash_entry ),
    ITEM( write_zero_address_crash ),
    ITEM( stack_overflow_crash ),
    ITEM( get_addr ),
};

void dump_all_func()
{
    int i = 0;

    for ( i=0; i<sizeof(func_list)/sizeof(func_list[0]); i++ ) {
        printf("func %s addr %p\n", func_list[i].str, func_list[i].addr );
    }
}

void file_test()
{
    FILE *fp = NULL;
    char *file = "/tmp/linking,v1,2akrar8tp7nww-ZGV2aWNlXzJha3Jhcjh0cGF2cnY=,frame,1545286757333-MTU0NTI4Njc1MDQ5Mw.jpg";
    char buffer[] = "test buffer";

    fp = fopen( file, "w+" );
    if ( !fp ) {
        LOGI("file open ok\n");
    }

    fwrite( buffer, 1, sizeof(buffer), fp );
    fclose( fp );
}

int main()
{
    dump_all_func();
    LOGI("enter main...\n");
//    crash_entry();
    file_test();
    LOGI("leave main...\n");
    
    return 0;
}



