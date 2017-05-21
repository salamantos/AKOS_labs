#include <sys/param.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

void rmSlashes( char* dest, char* src ) {
    // Обработка "/////////"
    char slash = src[0];
    int j = 0;
    dest[j++] = src[0];
    for (int i = 1; i < strlen( src ); ++i) {
        if (src[i] == slash && slash == '/') {

        } else {
            dest[j++] = src[i];
        }
        slash = src[i];
    }
    dest[j] = 0;
}

// Печатает путь по названию файла / по путю к нему
void printPath( char* arg ) {
    // Проверяем переходы вида ..
    char buffer[PATH_MAX];
    char buffer2[PATH_MAX];
    size_t i = 0, j = 0;
    char dot1 = 0, dot2 = 0; // Две точки
    i = strlen( arg ) - 1;
    int haveSlashBefore = 1;
    if (arg[strlen( arg ) - 1] == '.' && arg[strlen( arg ) - 2] == '.') {
        haveSlashBefore = 0;
    }
    while (i > 0) {
        dot1 = arg[i - 1];
        dot2 = arg[i];
        if (dot1 == '.' && dot2 == '.') {
            if (i < 3) {
                buffer[--j] = 0;
                break;
            }
            i -= 3;
            while (arg[i] != '/' && i > 0) {
                --i;
            }
            if (haveSlashBefore == 1) {
                --j;
            }
            if (i >= 0) buffer[j++] = arg[i];
        } else {
            buffer[j++] = arg[i];
            --i;
        }
        haveSlashBefore = 1;
    }
    int k = 0;
    int r = 0;
    for (k = j - 1; k >= 0; --k) {
        buffer2[r++] = buffer[k];
    }

    if (arg[0] == '/') {
        char temp[PATH_MAX];
        rmSlashes( temp, buffer2 );
        printf( "%s\n", temp );
    } else {
        char prePath[PATH_MAX]; // Путь от корня до данной директории
        char* res = getcwd( prePath, PATH_MAX );
        char temp[PATH_MAX];
        if (res != NULL) {
            sprintf( temp, "%s/%s\n", prePath, buffer2 );
            char temp2[PATH_MAX];
            rmSlashes( temp2, temp );
            printf( "%s", temp2 );
        } else {
            err( 1, "Error number %d", errno);
        }
    }
}

int main( int argc, char* argv[] ) {
    if (argc != 2) {
        fprintf( stderr, "realpath: omitted operand\n" );
        return 1;
    }

    // Обработка "/////////"
    char buffer[PATH_MAX];
    rmSlashes( buffer, argv[1] );

    printf( "%s\n", buffer );

    // Проверка, не является ли файл ссылкой
    struct stat sb;
    if (lstat( buffer, &sb ) == -1) {
        // No such file or directory
        if (errno != 2) {
            perror( "stat err" );
            exit( EXIT_FAILURE );
        }
    }
    // Если symlink
    if ((sb.st_mode & S_IFMT) == S_IFLNK) {
        char* linkName;
        linkName = (char*) malloc( PATH_MAX );
        readlink( argv[1], linkName, PATH_MAX ); // linkName - куда указывает ссылка
        linkName[sb.st_size] = '\0'; // Символ конца строки
        printPath( linkName );
    } else {
        printPath( buffer );
    }

    return 0;
}
