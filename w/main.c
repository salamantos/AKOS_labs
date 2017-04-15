#include <stdio.h>
#include <utmp.h>
#include <time.h>
#include <stdlib.h> // for getloadavg()

void printTime() {
    time_t rawTime;
    struct tm *timeInfo;
    char buffer[100];

    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(buffer, sizeof(buffer), "%X", timeInfo);
    printf("%s", buffer);
}

void printUpTime() {
    FILE *f;
    int x;
    f = fopen("/proc/uptime", "r");
    fscanf(f, "%d", &x);
    fclose(f);
    x /= 60;
    printf("%d", x);
}

void printUsersCount() {
    FILE *f;
    int usersCount = 0;
    struct utmp usr;
    f = fopen(_PATH_UTMP, "r");
    while (fread((char *) &usr, sizeof(usr), 1, f) == 1) {
        if (*usr.ut_name && *usr.ut_line && *usr.ut_line != '~') {
            usersCount++;
        }
    }
    printf("%d", usersCount - 1);
}

void printLoadAverage() {
    double load[3];
    if (getloadavg(load, 3) != -1) {
        printf("%7.6f , %7.6f , %7.6f", load[0], load[1], load[2]);
    }
}

void printUserInfo() {
    FILE *f;
    struct utmp usr;
    f = fopen(_PATH_UTMP, "r");
    while (fread((char *) &usr, sizeof(usr), 1, f) == 1) {
        if (*usr.ut_name && *usr.ut_line && *usr.ut_line != '~') {
            if (usr.ut_line[3] != '1') {
                printf("%1.8s", usr.ut_name);
                printf(" ");
                printf("%s", usr.ut_line);
                printf("     ");
                printf("%s", usr.ut_host);
                printf("               ");

                char buffer[80];
                struct tm *info;
                time_t timestamp = usr.ut_tv.tv_sec;
                info = localtime(&timestamp);
                strftime(buffer, 80, "%R", info);

                printf("%5.5s", buffer);
                printf("    ");
                printUpTime();
                printf("m");

                printf("\n");
            }
        }
    }
}

int main() {
    printf(" ");
    printTime();
    printf(" up ");
    printUpTime();
    printf(" min,  ");
    printUsersCount();
    printf(" user,  load average: ");
    printLoadAverage();
    printf("\nUSER     TTY      FROM             LOGIN@   IDLE   JCPU   PCPU WHAT\n");
    printUserInfo();

    return 0;
}