//  This file is distributed under the BSD 3-Clause License. See LICENSE for details.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <cxxabi.h>
#include <ctype.h>
#include <limits.h>

#include <iostream>

void I_gdb_continuation() {
  // Nothing to do here, just to allow a breakpoint for easier debug. Call:
  //
  // b I_gdb_continuation
  // c
  //
  // If you have a failing assertion
}


static bool is_gdb_attached() {

  char buf[PATH_MAX];

  const int status_fd = ::open("/proc/self/status", O_RDONLY);
  if (status_fd == -1)
    return false;

  const ssize_t num_read = ::read(status_fd, buf, sizeof(buf) - 1);
  if (num_read <= 0)
    return false;

  buf[num_read] = '\0';
  constexpr char tracerPidString[] = "TracerPid:";
  const auto tracer_pid_ptr = ::strstr(buf, tracerPidString);
  if (!tracer_pid_ptr)
    return false;

  for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read; ++characterPtr) {
    if (::isspace(*characterPtr))
      continue;
    return ::isdigit(*characterPtr) != 0 && *characterPtr != '0';
  }

  return false;
}

static bool try_gdb() {
  const char *I_GDB = getenv("I_GDB");
  if (I_GDB==0)
    return false;

  if (I_GDB[0] == '1') {
    char contents[10] = {0,};
    int fd = open("/proc/sys/kernel/yama/ptrace_scope", O_RDONLY);
    bool ptrace_works = true;
    if (fd>=0) {
      int len = read(fd,contents,1);
      if (len == 1 && contents[0] == '1') {
        fprintf(stderr,"ERROR: remote gdb not allowed. Set /proc/sys/kernel/yama/ptrace_scope to 0\n");
        fprintf(stderr,"echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope\n");
        fprintf(stderr,"to persist: sudo vim /etc/sysctl.d/10-ptrace.conf\n");
        fprintf(stderr,"   and set: kernel.yama.ptrace_scope = 0\n");
        ptrace_works = false;
      }
      close(fd);
    }

    if (ptrace_works) {
      for(int i=0;i<500;i++) { // At most 500 seconds waiting for gdb
        fprintf(stderr,"waiting for gdb, use: gdb -p %d\n", (int)getpid());
        sleep(i/4+1);
        if (is_gdb_attached())
          return true;
      }
    }
  }else if (I_GDB[0] != '0') {
    fprintf(stderr,"ERROR: unexpected I_GDB value. Either 1 or 0\n");
  }

  return false;
}

void I_internal(const char *file, int line, const char *condition, const char *message) {
  fprintf(stderr,"%s:%d :assertion %s failed\n", file, line, condition);
  if (message)
    fprintf(stderr,"%s\n",message);

  bool in_gdb = try_gdb();

  if (!in_gdb)
    abort();
}

#ifdef __GLIBC__
#include <execinfo.h>
typedef struct _sig_ucontext {
  unsigned long     uc_flags;
  struct ucontext   *uc_link;
  stack_t           uc_stack;
  struct sigcontext uc_mcontext;
  sigset_t          uc_sigmask;
} sig_ucontext_t;
#endif

void crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext) {

#ifdef __GLIBC__
  sig_ucontext_t * uc = (sig_ucontext_t *)ucontext;

  void * caller_address = (void *) uc->uc_mcontext.rip; // x86 specific

  fprintf(stderr,"signal %d (%s) address is 0x%p called from 0x%p\n", sig_num, strsignal(sig_num), info->si_addr, caller_address);

  void * array[50];
  int size = backtrace(array, 50);

  array[1] = caller_address;

  char ** messages = backtrace_symbols(array, size);

  // skip first stack frame (points here)
  for (int i = 1; i < size && messages != NULL; ++i) {
    char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

    // find parantheses and +address offset surrounding mangled name
    for (char *p = messages[i]; *p; ++p) {
      if (*p == '(') {
        mangled_name = p;
      } else if (*p == '+') {
        offset_begin = p;
      } else if (*p == ')') {
        offset_end = p;
        break;
      }
    }

    // if the line could be processed, attempt to demangle the symbol
    if (mangled_name && offset_begin && offset_end &&
        mangled_name < offset_begin) {
      *mangled_name++ = '\0';
      *offset_begin++ = '\0';
      *offset_end++ = '\0';

      int status;
      char * real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

      // if demangling is successful, output the demangled function name
      if (status == 0) {
        fprintf(stderr,"[backtrace]:%d %s:%s+%s%s\n",i,messages[i],real_name, offset_begin, offset_end);
      } else {
        fprintf(stderr,"[backtrace]:%d %s:%s+%s%s\n",i,messages[i],mangled_name, offset_begin, offset_end);
      }
      free(real_name);
    }else{
      fprintf(stderr,"[backtrace]:%d %s:\n",i,messages[i]);
    }
  }

  free(messages);
#endif

  bool in_gdb = try_gdb();
  if (!in_gdb)
    exit(EXIT_FAILURE);
}

void I_setup() {

  static int called = 0;
  if (called)
    return;
  called = true;

  struct sigaction sigact;

  sigact.sa_sigaction = crit_err_hdlr;
  sigact.sa_flags = SA_RESTART | SA_SIGINFO;

  if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0) {
    fprintf(stderr, "error setting signal handler for %d (%s)\n",
        SIGSEGV, strsignal(SIGSEGV));

    exit(EXIT_FAILURE);
  }
}
