#include <sys/param.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char buff[PATH_MAX];
    char *p;

    if (argc == 2) {
        if ((p = realpath(argv[1], buff)) == NULL) {
            if (errno == 2) {
                //No such file or directory
                if (argv[1][0] == '/') {
                    printf("%s\n", argv[1]);
                } else {
                    char buff1[PATH_MAX];
                    char *res = getcwd(buff1, PATH_MAX);
                    if (res != NULL) {
                        printf("%s%s%s\n", buff1, "/", argv[1]);
                    }
                }
            } else {
                err(1, "%s", buff);
            }
        } else {
            printf("%s\n", p);
        }
    } else {
        fprintf(stderr, "realpath: omitted operand\n");
        exit(1);
    }
    exit(0);
}

