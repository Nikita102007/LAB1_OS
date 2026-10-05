#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

void error(const char *msg)
{
    const char *p = msg;
    while (*p) p++;
    write(2, msg, p - msg);
    write(2, "\n", 1);
}

int main()
{
    char filename[100];
    int fd[2];
    ssize_t n = read(0, filename, sizeof(filename) - 1);
    
    if (n == -1) { 
        error("read error"); 
        return 1; 
    }

    if (n == 0) { 
        error("empty input"); 
        return 1; 
    }

    if (filename[n - 1] == '\n') filename[n - 1] = '\0';
    else filename[n] = '\0';

    if (pipe(fd) == -1) { 
        error("pipe error"); 
        return 1; 
    }

    pid_t pid = fork();

    if (pid == -1) {
        error("fork error"); 
        close(fd[0]); 
        close(fd[1]); 
        return 1;
    }
    if (pid == 0) {
        if (close(fd[1]) == -1) { 
            error("close error");
            _exit(1); 
        }
        if (dup2(fd[0], 0) == -1) { 
            error("dup2 error");
             _exit(1); 
        }
        if (close(fd[0]) == -1) { 
            error("close error"); 
            _exit(1); 
        }
        execl("./child", "child", filename, NULL);
        error("execl error"); 
        _exit(1);
    }

    if (close(fd[0]) == -1) { 
        error("close error");
        return 1; 
    }

    char buffer[100]; 
    char c = '\n';
    
    while (1) {
        int i = 0;
        int eof_reached = 0;
        while (i < (int)sizeof(buffer) - 1) {
            ssize_t r = read(0, &c, 1);
            if (r == -1) {
                if (errno == EINTR) continue;
                error("read error"); 
                close(fd[1]); 
                waitpid(pid, NULL, 0); 
                return 1;
            }
            if (r == 0) {
                eof_reached = 1;
                break;
            }
            if (c == '\n') break;
            buffer[i++] = c;
        }
        
        if (eof_reached && i == 0) {
            break;
        }
        
        if (i > 0) {
            if (write(fd[1], buffer, i) == -1) {
                error("write error"); 
                close(fd[1]); 
                waitpid(pid, NULL, 0); 
                return 1;
            }
        }
        
        if (!eof_reached || i > 0) {
            if (write(fd[1], "\n", 1) == -1) {
                error("write error"); 
                close(fd[1]); 
                waitpid(pid, NULL, 0); 
                return 1;
            }
        }
        
        if (eof_reached) {
            break;
        }
    }
    
    if (close(fd[1]) == -1) { 
        error("close error"); 
        waitpid(pid, NULL, 0); 
        return 1; 
    }
    if (waitpid(pid, NULL, 0) == -1) { 
        error("waitpid error"); 
        return 1; 
    }
    return 0;
}