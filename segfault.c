// Last Update:2019-03-15 11:49:54
/**
 * @file segfault.c
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-03-15
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

static void catch_segfault ( int signal, siginfo_t *_si, void *ctx )
{
    unsigned long int *addr = NULL;
    char * registers[] =
    {
        "trap_no",
        "error_code",
        "oldmask",
        "arm_r0",
        "arm_r1",
        "arm_r2",
        "arm_r3",
        "arm_r4",
        "arm_r5",
        "arm_r6",
        "arm_r7",
        "arm_r8",
        "arm_r9",
        "arm_r10",
        "arm_fp",
        "arm_ip",
        "arm_sp",
        "arm_lr",
        "arm_pc",
        "arm_cpsr",
        "fault_address",
    };   
    int i = 0;

    if ( signal == SIGSEGV ) {
        printf("signal : SIGSEGV\n");
        if ( _si ) {
            printf("signo : %d\n", _si->si_signo );
            printf("errno : %d\n", _si->si_errno );
            printf("addr : %p\n", _si->si_addr );
            printf("pid : %p\n", _si->si_pid );
        } else {
            printf("_si is NULL\n");
        }

        if ( ctx ) {
            addr = ( unsigned long int *)(ctx + sizeof(unsigned long ) + sizeof( void *) + sizeof( stack_t ));
            for ( i=0; i<sizeof(registers)/sizeof(registers[0]); i++ ) {
                printf("%s : 0x%lx\n", registers[i], *addr++ );
            }
        } else {
            printf("ctx is NULL\n");
        }
    }
    exit(1);
}


static void __attribute__ ((constructor)) install_handler (void)
{
    struct sigaction sa;
    stack_t ss;
    void *stack_mem = malloc (2 * SIGSTKSZ);

    sa.sa_handler = (void *) catch_segfault;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);


    if (stack_mem != NULL) {
        ss.ss_sp = stack_mem;
        ss.ss_flags = 0;
        ss.ss_size = 2 * SIGSTKSZ;

        if (sigaltstack (&ss, NULL) == 0)
            sa.sa_flags |= SA_ONSTACK;
    }

    sigaction (SIGSEGV, &sa, NULL);

}
