#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

void printInfoFromFile(char *fileName) {
    int pid = 0; // process id
    char tcomm[100]; // filename of the executable
    int tty_nr = 0;
    int tty_pgrp = 0;
    int utime = 0;
    int stime = 0;
    FILE *f = fopen(fileName, "r");
    if (f == NULL) {
        // Файл не удалось открыть
        printf("%s", "Error");
    } else {
        // Работа с файлом
        int number = 0;
        char string[100];
        fscanf(f, "%d", &pid);
        fscanf(f, "%s", tcomm);
        fscanf(f, "%s", string);
        int i;
        for (i = 0; i < 3; ++i) {
            fscanf(f, "%d", &number);
        }

        fscanf(f, "%d", &tty_nr);
        fscanf(f, "%d", &tty_pgrp);
        for (i = 0; i < 5; ++i) {
            fscanf(f, "%d", &number);
        }
        fscanf(f, "%d", &utime);
        fscanf(f, "%d", &stime);

        int time = (utime + stime) / 100; // In seconds
        int timeM = time / 60;
        int timeS = time % 60;
        // Удаляем () из строки
        size_t lenStr = strlen(tcomm);
        char procName[100];
        size_t j;
        for (j = 1; j < lenStr - 1; ++j) {
            procName[j - 1] = tcomm[j];
        }
        //printf("%s, %s\n", tcomm, procName);
        printf("%5.0d %-11.11s %2.2d:%2.2d %-10.50s\n", pid, "?", timeM, timeS, tcomm);
    }
}

int main() {
    printf("  PID TTY          TIME CMD\n");
    // Просматриваем все директории процессов
    DIR *dir = opendir("/proc");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            // Выбираем только директории, имена которых - числа
//            char nameD[100];
//            *nameD = *ent->d_name;
            size_t lenStr = strlen(ent->d_name);
            int isNumber = 1;
            size_t i;
            for (i = 0; i < lenStr; ++i) {
                if (!isdigit(ent->d_name[i])) {
                    isNumber=0;
                    break;
                }
            }
            if(isNumber==1){
                char way[100] = "/proc/";
                strcat(way, ent->d_name);
                strcat(way,"/stat");

                printInfoFromFile(way);
            }
        }
    } else {
        fprintf(stderr, "Error opening directory\n");
    }

    return 0;
}