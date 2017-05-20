#include <sys/param.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

// Печатает путь по названию файла / путю к нему
void printPath(char *arg) {
    if (arg[0] == '/') {
        printf("%s\n", arg);
    } else {
        char prePath[PATH_MAX]; // Путь от корня до данной директории
        char *res = getcwd(prePath, PATH_MAX);
        if (res != NULL) {
            printf("%s/%s\n", prePath, arg);
        } else {
            err(1, "Error number %d", errno);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "realpath: omitted operand\n");
        return 1;
    }

    // Проверка, не является ли файл ссылкой
    struct stat sb;
    if (lstat(argv[1], &sb) == -1) {
        // No such file or directory
        if (errno != 2) {
            perror("stat err");
            exit(EXIT_FAILURE);
        }
    }
    // Если symlink
    if ((sb.st_mode & S_IFMT) == S_IFLNK) {
        char *linkName;
        linkName = malloc(PATH_MAX);
        readlink(argv[1], linkName, PATH_MAX); // linkName - куда указывает ссылка
        linkName[sb.st_size] = '\0'; // Символ конца строки
        printPath(linkName);
    } else {
        printPath(argv[1]);
    }

    return 0;
}

