
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <android/log.h>


#define LOG_TAG "fake-sh"
#define LOGD(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


#define DEFAULT_SHELL       "/system/bin/sh"
#define DEFAULT_BUF_SIZE    1024

const char *g_argv[4] = { nullptr };

int pid = -1;
int opid = -1;
int epid = -1;

void sig_handler(int signo, siginfo_t *info, void *context) {
    // LOGD("receiver:signo %d", signo);
    kill(opid, SIGKILL);
    kill(epid, SIGKILL);
    exit(0);
}

int main(int argc, char *argv[]) {

    g_argv[0] = DEFAULT_SHELL;
    for (int i = 1; i < argc; i++) {
        LOGD("[+] i-%d %s", i, argv[i]);
        g_argv[i] = argv[i];
    }

    struct sigaction act = { 0 };
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sig_handler;
    if (sigaction(SIGCHLD, &act, NULL) == -1) {
        LOGD("Signal error: %d", errno);
        return -1;
    }

    int pipe_in[2];
    int pipe_out[2];
    int pipe_err[2];

    // Create pipes
    if (pipe(pipe_in) < 0) {
        LOGD("Create pipe error:%d", errno);
        return -1;
    }
    if (pipe(pipe_out) < 0) {
        LOGD("Create pipe error:%d", errno);
        return -1;
    }
    if (pipe(pipe_err) < 0) {
        LOGD("Create pipe error:%d", errno);
        return -1;
    }

    pid = fork();
    if (pid == 0) {
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[1]);
        
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[0]);

        dup2(pipe_err[1], STDERR_FILENO);
        close(pipe_err[0]);
        execvp(DEFAULT_SHELL, (char *const *)g_argv);
    } else if (pid > 0) {
        close(pipe_in[0]);
        close(pipe_out[1]);
        close(pipe_err[1]);

        if (opid == -1) {
            opid = fork();
            if (!opid) {
                close(pipe_err[0]);
                // LOGD("[+]Create stdout reading child process %d", getpid());
                char result[DEFAULT_BUF_SIZE] = {0};
                int num_read = 0;
                while ((num_read = read(pipe_out[0], &result, DEFAULT_BUF_SIZE - 1)) > 0) {
                    result[num_read] = '\0';
                    write(STDOUT_FILENO, result, num_read);
                    LOGD("[+]Read out %d-%s",num_read, result);
                }
                return 0;
            }
        }

        if (epid == -1) {
            epid = fork();
            if (!epid) {
                close(pipe_out[0]);
                // LOGD("[+]Create stderr reading child process: %d", getpid());
                char result[DEFAULT_BUF_SIZE] = {0};
                int num_read = 0;
                while ((num_read = read(pipe_err[0], &result, DEFAULT_BUF_SIZE - 1)) > 0) {
                    result[num_read] = '\0';
                    write(STDERR_FILENO, result, num_read);
                    LOGD("[+]Read err %d-%s",num_read, result);
                }
                return 0;
            }
        }

        char command[DEFAULT_BUF_SIZE] = {0};
        while (fgets((char *)&command, DEFAULT_BUF_SIZE, stdin)) {
            int len = strlen(command);
            LOGD("[+]Read command: %d-%s", strlen(command), &command);

            // Special case
            /*if (len > 2 && command[len - 2] == 0x20 && command[len-1] == 0xA) {
                command[len-2] = 0xA;
                command[len-1] = '\0';
            }*/

            write(pipe_in[1], &command, strlen(command) + 1);
        }
    }
}
