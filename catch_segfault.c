// Last Update:2018-12-20 16:08:36
/**
 * @file segfault.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-12-19
 */

/* Catch segmentation faults and print backtrace.
   Copyright (C) 1998-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/uio.h>
#include <sys/ucontext.h>

/* We'll use this a lot.  */
#define WRITE_STRING(s) write (fd, s, strlen (s))
#define GET_PC(ctx)        ((void *) (ctx)->uc_mcontext.arm_pc)

/* Name of the output file.  */
static const char *fname;
static const char _itoa_lower_digits[16] = "0123456789abcdef";
static const char _itoa_upper_digits[16] = "0123456789ABCDEF";
static inline char * __attribute__ ((unused, always_inline))

_itoa_word (unsigned long value, char *buflim,
            unsigned int base, int upper_case)
{
  const char *digits = (upper_case ? _itoa_upper_digits : _itoa_lower_digits);
  switch (base)
    {
# define SPECIAL(Base)                                                              \
    case Base:                                                                      \
      do                                                                      \
        *--buflim = digits[value % Base];                                      \
      while ((value /= Base) != 0);                                              \
      break
      SPECIAL (10);
      SPECIAL (16);
      SPECIAL (8);
    default:
      do
        *--buflim = digits[value % base];
      while ((value /= base) != 0);
    }
  return buflim;
}
static inline char * __attribute__ ((unused, always_inline))
_itoa (uint64_t value, char *buflim,
       unsigned int base, int upper_case)
{
  const char *digits = (upper_case ? _itoa_upper_digits : _itoa_lower_digits);
  switch (base)
    {
      SPECIAL (10);
      SPECIAL (16);
      SPECIAL (8);
    default:
      do
        *--buflim = digits[value % base];
      while ((value /= base) != 0);
    }
  return buflim;
}
# undef SPECIAL


/* We will print the register dump in this format:
 R0: XXXXXXXX   R1: XXXXXXXX   R2: XXXXXXXX   R3: XXXXXXXX
 R4: XXXXXXXX   R5: XXXXXXXX   R6: XXXXXXXX   R7: XXXXXXXX
 R8: XXXXXXXX   R9: XXXXXXXX   SL: XXXXXXXX   FP: XXXXXXXX
 IP: XXXXXXXX   SP: XXXXXXXX   LR: XXXXXXXX   PC: XXXXXXXX
 CPSR: XXXXXXXX
 Trap: XXXXXXXX   Error: XXXXXXXX   OldMask: XXXXXXXX
 Addr: XXXXXXXX
 */

static void
hexvalue (unsigned long int value, char *buf, size_t len)
{
  char *cp = _itoa_word (value, buf + len, 16, 0);
  while (cp > buf)
    *--cp = '0';
}
static void
register_dump (int fd, const struct ucontext *ctx)
{
  char regs[21][8];
  struct iovec iov[97];
  size_t nr = 0;

  printf("dump registers\n");
#define ADD_STRING(str) \
  iov[nr].iov_base = (char *) str;					      \
  iov[nr].iov_len = strlen (str);					      \
  ++nr
#define ADD_MEM(str, len) \
  iov[nr].iov_base = str;						      \
  iov[nr].iov_len = len;						      \
  ++nr

  /* Generate strings of register contents.  */
  hexvalue (ctx->uc_mcontext.arm_r0, regs[0], 8);
  hexvalue (ctx->uc_mcontext.arm_r1, regs[1], 8);
  hexvalue (ctx->uc_mcontext.arm_r2, regs[2], 8);
  hexvalue (ctx->uc_mcontext.arm_r3, regs[3], 8);
  hexvalue (ctx->uc_mcontext.arm_r4, regs[4], 8);
  hexvalue (ctx->uc_mcontext.arm_r5, regs[5], 8);
  hexvalue (ctx->uc_mcontext.arm_r6, regs[6], 8);
  hexvalue (ctx->uc_mcontext.arm_r7, regs[7], 8);
  hexvalue (ctx->uc_mcontext.arm_r8, regs[8], 8);
  hexvalue (ctx->uc_mcontext.arm_r9, regs[9], 8);
  hexvalue (ctx->uc_mcontext.arm_r10, regs[10], 8);
  hexvalue (ctx->uc_mcontext.arm_fp, regs[11], 8);
  hexvalue (ctx->uc_mcontext.arm_ip, regs[12], 8);
  hexvalue (ctx->uc_mcontext.arm_sp, regs[13], 8);
  hexvalue (ctx->uc_mcontext.arm_lr, regs[14], 8);
  hexvalue (ctx->uc_mcontext.arm_pc, regs[15], 8);
  hexvalue (ctx->uc_mcontext.arm_cpsr, regs[16], 8);
  hexvalue (ctx->uc_mcontext.trap_no, regs[17], 8);
  hexvalue (ctx->uc_mcontext.error_code, regs[18], 8);
  hexvalue (ctx->uc_mcontext.oldmask, regs[19], 8);
  hexvalue (ctx->uc_mcontext.fault_address, regs[20], 8);
  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\n R0: ");
  ADD_MEM (regs[0], 8);
  ADD_STRING ("   R1: ");
  ADD_MEM (regs[1], 8);
  ADD_STRING ("   R2: ");
  ADD_MEM (regs[2], 8);
  ADD_STRING ("   R3: ");
  ADD_MEM (regs[3], 8);
  ADD_STRING ("\n R4: ");
  ADD_MEM (regs[4], 8);
  ADD_STRING ("   R5: ");
  ADD_MEM (regs[5], 8);
  ADD_STRING ("   R6: ");
  ADD_MEM (regs[6], 8);
  ADD_STRING ("   R7: ");
  ADD_MEM (regs[7], 8);
  ADD_STRING ("\n R8: ");
  ADD_MEM (regs[8], 8);
  ADD_STRING ("   R9: ");
  ADD_MEM (regs[9], 8);
  ADD_STRING ("   SL: ");
  ADD_MEM (regs[10], 8);
  ADD_STRING ("   FP: ");
  ADD_MEM (regs[11], 8);
  ADD_STRING ("\n IP: ");
  ADD_MEM (regs[12], 8);
  ADD_STRING ("   SP: ");
  ADD_MEM (regs[13], 8);
  ADD_STRING ("   LR: ");
  ADD_MEM (regs[14], 8);
  ADD_STRING ("   PC: ");
  ADD_MEM (regs[15], 8);
  ADD_STRING ("\n\n CPSR: ");
  ADD_MEM (regs[16], 8);
  ADD_STRING ("\n\n Trap: ");
  ADD_MEM (regs[17], 8);
  ADD_STRING ("   Error: ");
  ADD_MEM (regs[18], 8);
  ADD_STRING ("   OldMask: ");
  ADD_MEM (regs[19], 8);
  ADD_STRING ("\n Addr: ");
  ADD_MEM (regs[20], 8);
  ADD_STRING ("\n");
  /* Write the stuff out.  */
  writev (fd, iov, nr);
}

#define REGISTER_DUMP register_dump (fd, ctx)

/* This function is called when a segmentation fault is caught.  The system
   is in an unstable state now.  This means especially that malloc() might
   not work anymore.  */
static void
catch_segfault (int signal, struct ucontext *ctx)
{
  int fd, cnt, i;
  void **arr;
  struct sigaction sa;
  uintptr_t pc;

  printf("get signal %d\n", signal );

  /* This is the name of the file we are writing to.  If none is given
     or we cannot write to this file write to stderr.  */
  fd = 2;
  if (fname != NULL)
    {
      fd = open (fname, O_TRUNC | O_WRONLY | O_CREAT, 0666);
      if (fd == -1)
	fd = 2;
    }

  WRITE_STRING ("*** ");
  //write_strsignal (fd, signal);
  WRITE_STRING ("\n");

  REGISTER_DUMP;

  WRITE_STRING ("\nBacktrace:\n");

  /* Get the backtrace.  */
  arr = alloca (256 * sizeof (void *));
  cnt = backtrace (arr, 256);

  /* Now try to locate the PC from signal context in the backtrace.
     Normally it will be found at arr[2], but it might appear later
     if there were some signal handler wrappers.  Allow a few bytes
     difference to cope with as many arches as possible.  */
  pc = (uintptr_t) GET_PC (ctx);
  printf("pc = 0x%p\n", pc );
  printf("pc = 0x%x\n", pc );

  for (i = 0; i < cnt; ++i)
    if ((uintptr_t) arr[i] >= pc - 16 && (uintptr_t) arr[i] <= pc + 16)
      break;

  /* If we haven't found it, better dump full backtrace even including
     the signal handler frames instead of not dumping anything.  */
  if (i == cnt)
    i = 0;

  /* Now generate nicely formatted output.  */
  __backtrace_symbols_fd (arr + i, cnt - i, fd);


  /* Now the link map.  */
  int mapfd = open ("/proc/self/maps", O_RDONLY);
  if (mapfd != -1)
    {
      write (fd, "\nMemory map:\n\n", 14);

      char buf[256];
      ssize_t n;

      while ((n = read (mapfd, buf, sizeof (buf))))  
          write (fd, buf, n);

      close (mapfd);
    }

  /* Pass on the signal (so that a core file is produced).  */
  sa.sa_handler = SIG_DFL;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction (signal, &sa, NULL);
  raise (signal);
}


static void
__attribute__ ((constructor))
install_handler (void)
{
  struct sigaction sa;
  const char *name;

  sa.sa_handler = (void *) catch_segfault;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  printf("install handler \n");
  /* Maybe we are expected to use an alternative stack.  */
  {
      void *stack_mem = malloc (2 * SIGSTKSZ);
      stack_t ss;

      if (stack_mem != NULL)
      {
          ss.ss_sp = stack_mem;
          ss.ss_flags = 0;
          ss.ss_size = 2 * SIGSTKSZ;

          if (sigaltstack (&ss, NULL) == 0)
              sa.sa_flags |= SA_ONSTACK;
      }
  }

  sigaction (SIGSTKFLT, &sa, NULL);
  sigaction (SIGSEGV, &sa, NULL);
  sigaction (SIGILL, &sa, NULL);
  sigaction (SIGBUS, &sa, NULL);
  sigaction (SIGABRT, &sa, NULL);
  sigaction (SIGFPE, &sa, NULL);
}

