#include "UsersModule.h"
#include <string.h>
#include <regex.h>
#include <dirent.h>
#include "Common.h"

/*
Информация о пользователе хранится в отдельном файле
Формат файла следующий:
    login\n
    password\n
    isKiked\n (bool - 0/1)
*/

char* hash( char* str ) {
    return str;
}

int isCorrect( char* login, char* password ) {
    if (strcmp( login, "root" ) == 0 && strcmp( password, ROOT_PASSWORD ) != 0) {
        return 5;
    }
    if (strlen( login ) > 32 || strlen( password ) > 32 || strlen( password ) == 0) {
        return 4;
    }
    for (int i = 0; i < strlen( login ); ++i) {
        if (!((login[i] <= 'z' && login[i] >= 'A') || (login[i] <= '9' && login[i] >= '0'))) {
            return 4;
        }
    }
    for (int i = 0; i < strlen( password ); ++i) {
        if (!((password[i] <= 'z' && password[i] >= 'A') || (password[i] <= '9' && password[i] >= '0'))) {
            return 4;
        }
    }
    return 0;
}

int isUserExist( struct CUser* usersList, char* login ) {
    for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
        if (usersList[i].id != 0) {
            if (strcmp( usersList[i].login, login ) == 0) {
                return 1; // Есть пользователь
            }
        }
    }
    // Нет такого пользователя
    return 0;
}

int readUserData( struct CUser* usersList, char* login, char* getPassword, int* getIsKicked ) {
    for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
        if (strcmp( usersList[i].login, login ) == 0) {
            strcpy( getPassword, usersList[i].password );
            *getIsKicked = usersList[i].isKicked;
            return 0; // Есть пользователь
        }
    }
    return 1;
}

int authentication( struct CUser* usersList, char* login, char* password, int sockfd ) {
    // == 1 - пользователь не существует
    // == 2 - неверный пароль
    // == 0 - аутентификация пройдена
    // == 5 - kicked
    // == 6 - unknown error
    int res = isUserExist( usersList, login );
    if (res == 1) {
        char getPassword[100];
        int getIsKicked;
        readUserData( usersList, login, getPassword, &getIsKicked );
        if (!strcmp( getPassword, password )) {
            // Делаем активным
            for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
                if (usersList[i].id != 0) {
                    if (strcmp( usersList[i].login, login ) == 0) {
                        if (usersList[i].isKicked == 1) {
                            return 5;
                        }
                        usersList[i].isOnline = 1;
                        usersList[i].sockfd = sockfd;
                        return 0;
                    }
                }
            }
            return 0;
        } else {
            return 2;
        }
    } else {
        if (res == 0) {
            return 1;
        } else {
            return res;
        }
    }
}

int createUser( struct CUser* usersList, char* login, char* password, int isKicked, int sockfd ) {
    if (isUserExist( usersList, login ) == 1) {
        // Изменяем сокет
        for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
            if (strcmp( usersList[i].login, login ) == 0) {
                usersList[i].sockfd = sockfd;
                return 0;
            }
        }
    }

    // Проверяем логин и пароль на коректность
    int status = isCorrect( login, password );
    if (status != 0) {
        return status;
    }

    struct CUser userInfo;
    userInfo.login = login;
    userInfo.password = password;
    userInfo.id = userIdRandom++;
    userInfo.isOnline = 1;
    userInfo.isKicked = isKicked;
    userInfo.sockfd = sockfd;
    addUserToList( usersList, &userInfo );

    return 0;
}

int kick( struct CUser* usersList, int id, int* kickedSock, int* isOnline ) {
    // == 1 - пользователь не существует
    // == 0 - успешно
    // == 6 - unknown error
    // Кикаем
    for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
        if (usersList[i].id == id) {
            usersList[i].isKicked = 1;
            *isOnline = usersList[i].isOnline;
            usersList[i].isOnline = 0;
            *kickedSock = usersList[i].sockfd;
            return 0;
        }
    }
    return 1;
}