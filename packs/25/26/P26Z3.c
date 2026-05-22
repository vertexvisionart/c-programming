#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *syscall_name(long nr) {
    switch (nr) {
        case 0: return "read";
        case 1: return "write";
        case 2: return "open";
        case 3: return "close";
        case 4: return "stat";
        case 5: return "fstat";
        case 6: return "lstat";
        case 7: return "poll";
        case 8: return "lseek";
        case 9: return "mmap";
        case 10: return "mprotect";
        case 11: return "munmap";
        case 12: return "brk";
        case 13: return "rt_sigaction";
        case 14: return "rt_sigprocmask";
        case 16: return "ioctl";
        case 17: return "pread64";
        case 18: return "pwrite64";
        case 21: return "access";
        case 22: return "pipe";
        case 23: return "select";
        case 24: return "sched_yield";
        case 32: return "dup";
        case 33: return "dup2";
        case 39: return "getpid";
        case 41: return "socket";
        case 42: return "connect";
        case 56: return "clone";
        case 57: return "fork";
        case 58: return "vfork";
        case 59: return "execve";
        case 60: return "exit";
        case 61: return "wait4";
        case 62: return "kill";
        case 63: return "uname";
        case 72: return "fcntl";
        case 79: return "getcwd";
        case 80: return "chdir";
        case 82: return "rename";
        case 83: return "mkdir";
        case 84: return "rmdir";
        case 85: return "creat";
        case 87: return "unlink";
        case 89: return "readlink";
        case 97: return "getrlimit";
        case 102: return "getuid";
        case 104: return "getgid";
        case 107: return "geteuid";
        case 108: return "getegid";
        case 158: return "arch_prctl";
        case 202: return "futex";
        case 217: return "getdents64";
        case 218: return "set_tid_address";
        case 228: return "clock_gettime";
        case 231: return "exit_group";
        case 257: return "openat";
        case 262: return "newfstatat";
        case 273: return "set_robust_list";
        case 302: return "prlimit64";
        case 318: return "getrandom";
        case 334: return "rseq";
        default: return "unknown";
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }

    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        return 1;
    }

    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        raise(SIGSTOP);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127);
    }

    int status = 0;
    if (waitpid(child, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD) < 0) {
        perror("ptrace SETOPTIONS");
        return 1;
    }

    int in_syscall = 0;
    long current_nr = -1;

    while (1) {
        if (ptrace(PTRACE_SYSCALL, child, NULL, NULL) < 0) {
            if (errno == ESRCH) {
                break;
            }
            perror("ptrace");
            return 1;
        }

        if (waitpid(child, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            break;
        }
        if (!WIFSTOPPED(status)) {
            continue;
        }
        int signal = WSTOPSIG(status);
        if (signal != (SIGTRAP | 0x80)) {
            continue;
        }

        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, child, NULL, &regs) < 0) {
            perror("ptrace GETREGS");
            return 1;
        }

        if (!in_syscall) {
            current_nr = (long)regs.orig_rax;
            in_syscall = 1;
        } else {
            long ret = (long)regs.rax;
            printf("syscall: %s (%ld) = %ld\n", syscall_name(current_nr),
                   current_nr, ret);
            in_syscall = 0;
        }
    }

    return 0;
}
