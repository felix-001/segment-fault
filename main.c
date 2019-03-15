
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASIC() printf("[ %s %s() +%d ] ", __FILE__, __FUNCTION__, __LINE__ )
#define LOGI(args...) BASIC();printf(args)
#define LOGE(args...) LOGI(args)

#define TEST_BUFER_OVERFLOW 0
#define TEST_NULL_POINTER 0
#define TEST_DOUBLE_FREE 0
#define TEST_WILD_POINTER 1

char *get_ptr()
{
    char buffer[10];

    return buffer;
}

void func_for_cover_stack()
{
    char buffer[1024] = { "9999999999999"};

    printf("buffer = %s\n", buffer );
};

void segfault_wild_pointer()
{
    char *p = NULL;

    p = get_ptr();
    func_for_cover_stack();

    memcpy( p, "1234567890", 20 );
}

void gen_double_free()
{
    char *p = (char *)malloc (1024);

    free( p );
    free( p );
}

void segfault_double_free()
{
    gen_double_free();
}

void gen_null_pointer_crash()
{
    char *p = (char *)0X12345678;

    *p = 0;
}

void segfault_null_pointer()
{
    gen_null_pointer_crash();
}

void gen_buffer_overflow()
{
    char buf[10];
    int i = 0;

    for ( i=0; i<50000; i++ ) {
        buf[i] = 0xaa;
    }
}

void segfault_buffer_overflow()
{
    gen_buffer_overflow();
}

int main()
{
    LOGI("enter main\n");

#if TEST_NULL_POINTER
    segfault_null_pointer();
#endif
#if TEST_BUFER_OVERFLOW
    segfault_buffer_overflow();
#endif
#if TEST_DOUBLE_FREE
    segfault_double_free();
#endif
#if TEST_WILD_POINTER
    segfault_wild_pointer();
#endif

    LOGI("leave main\n");

    return 0;
}
