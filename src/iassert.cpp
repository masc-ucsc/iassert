
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void I_internal(const char *file, int line, const char *condition, const char *message) {
  fprintf(stderr,"%s:%d :assertion %s failed\n", file, line, condition);
  if (message)
    fprintf(stderr,"%s\n",message);

  const char *I_GDB = getenv("I_GDB");
  if (I_GDB) {
    if (I_GDB[0] == '1') {
      char contents[10] = {0,};
      int fd = open("/proc/sys/kernel/yama/ptrace_scope", O_RDONLY);
      int ptrace_works = 1;
      if (fd>=0) {
        int len = read(fd,contents,1);
        if (len == 1 && contents[0] == '1') {
          fprintf(stderr,"ERROR: remote gdb not allowed. Set /proc/sys/kernel/yama/ptrace_scope to 0\n");
          fprintf(stderr,"echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope\n");
          fprintf(stderr,"to persist: sudo vim /etc/sysctl.d/10-ptrace.conf\n");
          fprintf(stderr,"   and set: kernel.yama.ptrace_scope = 0\n");
          ptrace_works = 0;
        }
        close(fd);
      }

      if (ptrace_works) {
        for(int i=0;i<500;i++) { // At most 500 seconds waiting for gdb
          fprintf(stderr,"waiting for gdb, use: gdb -p %d\n", (int)getpid());
          sleep(i/4+1);
        }
      }
    }else if (I_GDB[0] != '0') {
      fprintf(stderr,"ERROR: unexpected I_GDB value. Either 1 or 0\n");
    }
  }
  abort();
}

