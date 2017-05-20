#include "Common.h"
#include "TCP_connection.h"

void* newThread( void* param ) {
    connectNewUser((struct CThreadParam*) param );
};

int addUserToList( struct CUser* usersList, struct CUser* userInfo ) {
    for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
        if (usersList[i].id == 0) {
            usersList[i] = *userInfo;
            return 0;
        }
    }
    // Нет места
    return 1;
};

int rmUserFromList( struct CUser* usersList, char* login ) {
    for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
        if (strcmp( usersList[i].login, login ) == 0) {
            usersList[i].isOnline = 0;
            return 0;
        }
    }
    // Нет такого пользователя
    return 1;
};

void sendToAll( char* message, size_t messSize, int onlineCount, struct CUser* usersList ) {
    int i = 0; // Количество пользователей, получивших новое сообщение
    int j = 0; // Индекс массива
    while (i < onlineCount && i < MAX_CONNECTED_USERS) {
        if (usersList[j].id != 0 && usersList[j].isOnline == 1) {
            printf("\n----->");
            for (int k = 0; k < messSize; ++k) {
                printf("%c", message[k]);
            }
            printf("<-----\n");
            ssize_t n = write( usersList[j].sockfd, message, messSize );
            if (n < 0) {
                error( "ERROR writing to socket" );
            }
            ++i;
        }
        ++j;
    }
}