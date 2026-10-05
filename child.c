#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

void error(const char *msg)
{
    const char *p = msg;
    while (*p) {
        p++;
    }
    write(2, msg, p - msg);
    write(2, "\n", 1);
}

int main(int argc, char *argv[])
{
    if (argc < 2) { 
        error("filename is not specified"); 
        return 1; 
    }

    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) { 
        error("open error"); 
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
                if (errno == EINTR) {
                    continue;
                }
                error("read error"); 
                close(file); 
                return 1;
            }
            if (r == 0) {
                eof_reached = 1;
                break;
            }
            if (c == '\n') {
                break;
            }
            buffer[i++] = c;
        }

        if (eof_reached && i == 0) {
            break;
        }

        int sum = 0;
        int number = 0;
        int sign = 1;
        int in_number = 0;

        for (int j = 0; j <= i; j++) {
            if (j < i && buffer[j] == '-' && !in_number) {
                sign = -1; 
                in_number = 1; 
                number = 0;
            } else if (j < i && buffer[j] >= '0' && buffer[j] <= '9') {
                number = number * 10 + (buffer[j] - '0'); 
                in_number = 1;
            } else {
                if (in_number) {
                    sum += sign * number;
                }
                number = 0; 
                sign = 1; 
                in_number = 0;
            }
        }

        char result[50]; 
        int pos = 0;
        long long value = sum;

        if (value < 0) { 
            result[pos++] = '-'; 
            value = -value; 
        }

        char digits[20]; 
        int count = 0;

        do { 
            digits[count++] = '0' + (value % 10); 
            value /= 10; 
        } while (value > 0);

        while (count > 0) {
            result[pos++] = digits[--count];
        }
        result[pos++] = '\n';

        if (write(file, result, pos) == -1) {
            error("write error"); 
            close(file); 
            return 1;
        }

        if (eof_reached) {
            break;
        }
    }

    if (close(file) == -1) { 
        error("close error"); 
        return 1; 
    }

    return 0;
}