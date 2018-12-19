// Last Update:2018-12-19 23:42:53
/**
 * @file crash.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-12-19
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void func()
{
    char *p = NULL;

    *p = 10;
}

char *get_mem()
{
    char buffer[10];

    return buffer;
}

void stack_destroy_crash()
{
    char *p = get_mem();
    int i = 0;

    for ( i=0; i<10000; i++ )
        p[i] = 10;

}


void foo()
{
//    func();
    stack_destroy_crash();
}

void hello()
{
    printf("hello\n");
}

void dump_funcs();
void SignalHandler( int sig )
{
    dump_funcs();
    exit(0);
}


int main()
{
    signal( SIGSEGV, SignalHandler );
    printf("foo address %p\n", foo );
    printf("func address %p\n", func );
    printf("main address %p\n", main );
    printf("SignalHandler address %p\n", SignalHandler );
    printf("hello address %p\n", hello );
    printf("stack_destroy_crash address %p\n", stack_destroy_crash );
    printf("dump_funcs address %p\n", dump_funcs );
    printf("get_mem address %p\n", get_mem );
    hello();
    foo();
    return 0;
}

typedef struct {
    void *addr;
    int count;
} func_trace_t;

static func_trace_t funcs[1024];

static int current = 0;

#define CHECK_EXIST() \
    for ( i=0; i<current; i++ ) { \
        if( funcs[i].addr == this_func ) { \
            found = 1; \
            break; \
        } \
    }

void dump_funcs()
{
    int i = 0;

    printf("current = %d\n", current );
    for ( i=0; i<current; i++ ) {
        if ( funcs[i].count > 0 ) {
            printf("%p\n", funcs[i].addr );
        }
    }
}

void __attribute__((__no_instrument_function__))
    __cyg_profile_func_enter(void *this_func, void *call_site)
{
    int i = 0, found = 0;

    CHECK_EXIST();
    if ( found ) {
        funcs[i].count++;
    } else {
        funcs[current].addr = this_func;
        funcs[current].count++;
        current++;
    }
}

void __attribute__((__no_instrument_function__))
    __cyg_profile_func_exit(void *this_func, void *call_site)
{
    int i = 0,  found = 0;

    CHECK_EXIST();

    if ( found ) {
        funcs[i].count--;
    }
}

