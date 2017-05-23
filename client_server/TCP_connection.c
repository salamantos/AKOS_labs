#include "TCP_connection.h"
#include "Common.h"
#include "MessagesFormat.h"
#include "UsersModule.h"

// Отправляет историю сообщений пользователю
void sendHistory( int count, int sockfd, int lastMessId, struct CMessage* messBuffer ) {
    count = min( 50, count );
    for (int i = max( lastMessId - count, 0 ); i < lastMessId; ++i) {
        // В буфере уже сформированные сообщения для отправки
        messBuffer[i].mess[0] = 'h';
        ssize_t n = write( sockfd, messBuffer[i].mess, messBuffer[i].len );
        messBuffer[i].mess[0] = 'r';
        if (n < 0) error( "ERROR writing to socket" );
    }
}

int switchMessType( char type, char* messBody, size_t messBSize, char* getLogin, struct CThreadParam* param ) {
    // Получаем параметры
    int newsockfd = param->newsockfd;
    struct CMessage* messBuffer = param->messBuffer;
    int* lastMessId = param->lastMessId;
    int* onlineCount = param->onlineCount;
    struct CUser* usersList = param->usersList;
    sem_t* semaphore = param->semaphore;

    char message[MESSAGE_LEN];
    int sendAnswer = 0;
    char exitMess[100];
    int kickFromChat = 0;
    size_t temp = 0;
    char* lines[3];
    lines[0] = (char*) malloc( MESSAGE_LEN );
    lines[1] = (char*) malloc( MESSAGE_LEN );
    lines[2] = (char*) malloc( MESSAGE_LEN );
    getLinesList( messBody, messBSize, lines, &temp );
    char messageBody[MESSAGE_LEN];
    size_t messBLen = 0;
    switch (type) {
        case 'i':
            // Регистрация/авторизация
            sendAnswer = 1; // Отвечать пользователю на его сообщение или нет
            type = 's';
            int status = 0;
            // Получаем логин и пароль
            char* login = lines[0];
            char* password = lines[1];

            sem_wait( semaphore );
            // Проверяем, зареган юзер или нет
            int userStatus = isUserExist( usersList, login );
            sem_post( semaphore );
            if (userStatus == 0) {
                sem_wait( semaphore );
                // Пытаемся зарегистрировать
                int crStatus = createUser( usersList, login, password, 0, newsockfd );
                sem_post( semaphore );
                if (crStatus == 0) {
                    status = 0;
                } else if (crStatus == 4) {
                    status = 4; // Ошибка регистрации
                    kickFromChat = 1;
                } else if (crStatus == 5) {
                    status = 5; // Ошибка доступа
                    kickFromChat = 1;
                }
            } else if (userStatus == 1) {
                sem_wait( semaphore );
                // Пытаемся войти
                int auStatus = authentication( usersList, login, password, newsockfd );
                sem_post( semaphore );
                if (auStatus == 0) {
                    status = 0;
                } else if (auStatus == 5) {
                    status = 5; // kicked
                    kickFromChat = 1;
                } else {
                    status = 3; // Ошибка аутентификации
                    kickFromChat = 1;
                }
            }
            strcpy( getLogin, login );
            if (status == 0) {
                sem_wait( semaphore );
                (*onlineCount)++;
                sem_post( semaphore );
//                sprintf( exitMess, "%s joined the chat", login );
//                char message2[MESSAGE_LEN];
//                size_t messSize2 = formMessage( message2, 'm', exitMess, strlen( exitMess ));
//                sendToAll( message2, messSize2, *onlineCount, usersList );
            }
            bzero( messBody, 9 );
            messBody[3] = 4;
            messBody[7] = status;
            messBSize = 8;
            break;
        case 'r':
            // Обычное сообщение, добавляем к нему логин отправителя
            sendAnswer = 0;
            char* constMessBody = (char*) malloc( MESSAGE_LEN );

            lines[2] = lines[0];
            lines[0] = "1";
            lines[1] = getLogin;

            formMessageBody( messageBody, &messBLen, lines, 3 );
            messBSize = formMessage( constMessBody, 'r', messageBody, messBLen );

            // Сохраняем в буффер
            struct CMessage* newMess = (struct CMessage*) malloc( sizeof( struct CMessage ));
            newMess->mess = constMessBody;
            newMess->len = messBLen + 5;
            sem_wait( semaphore );
            messBuffer[*(lastMessId)] = *newMess;
            *lastMessId = *lastMessId + 1;
            sem_post( semaphore );
            break;
        case 'l':
            sendAnswer = 1;
            sem_wait( semaphore );
            prepareUsersList( messBody, &messBSize, onlineCount, usersList );
            sem_post( semaphore );
            break;
        case 'o':
            sprintf( exitMess, "%s left the chat", getLogin );
            char message[MESSAGE_LEN];
            size_t messSize1 = formMessage( message, 'm', exitMess, strlen( exitMess ));
            sendToAll( message, messSize1, *onlineCount, usersList );
            break;
        case 'k':
            sendAnswer = 1;
            int kickRes = 0;
            // Проверяем права доступа
            if (strcmp( getLogin, "root" ) != 0) {
                type = 's';
                bzero( messBody, MESSAGE_LEN );
                messBody[0] = '5'; // Ошибка доступа
                break;
            } else {
                int kickedSock;
                int isOnline = 0;
                // Кикаем и, если еще онлайн, отправляем сообщение
                kickRes = kick( usersList, atoi( lines[0] ), &kickedSock, &isOnline );
                if (isOnline == 1) {
                    lines[0] = lines[1];
                    formMessageBody( messageBody, &messBLen, lines, 1 );
                    size_t messSize = formMessage( message, 'k', messageBody, messBLen );
                    ssize_t n = write( kickedSock, message, messSize );
                    if (n < 0) {
                        error( "ERROR writing to socket" );
                    }
                }
            }
            // Сообщаем root-у о результате
            bzero( messBody, 9 );
            messBody[3] = 4;
            messBSize = 8;
            type = 's';
            if (kickRes == 0) {
                messBody[7] = 0;
            } else {
                messBody[7] = 6;
            }
            break;
        case 'h':
            sendHistory( atoi( lines[0] ), newsockfd, *lastMessId, messBuffer );
            break;
        default:
            sendAnswer = 1;
            bzero( messBody, 9 );
            messBody[7] = 1;
            messBSize = 8;
            type = 's';
            break;
    }

    if (sendAnswer == 1) {
        size_t messSize = formMessage( message, type, messBody, messBSize );
        ssize_t n = write( newsockfd, message, messSize );
        if (n < 0) {
            error( "ERROR writing to socket" );
        }
    }
    return kickFromChat;
}

int connectNewUser( struct CThreadParam* param ) {
    char buffer[MESSAGE_LEN];
    ssize_t n;

    // Получаем параметры
    int newsockfd = param->newsockfd;
    struct CMessage* messBuffer = param->messBuffer;
    int* lastMessId = param->lastMessId;
    int* onlineCount = param->onlineCount;
    struct CUser* usersList = param->usersList;
    sem_t* semaphore = param->semaphore;

    // Чтобы для каждого потока значения были разные, а не ссылались на одну структуру
    struct CThreadParam localParam;
    localParam = *param;

    printf( "Started talking\n" );
    char type = '0';
    char* login = (char*) malloc( 50 );
    while (type != 'o') {
        // Читаем сообщение
        bzero( buffer, MESSAGE_LEN );
        ssize_t n = read( newsockfd, buffer, 5 ); // 1 байт - тип. Следующие 4 - размер сообщения
        size_t bytesCount = bytesToInt( buffer + 1 ) - 5;
        n = read( newsockfd, buffer + 5, bytesCount );
        if (buffer[0] == 0) break; // Клиент отключился
        if (n < 0) error( "ERROR reading from socket" );

        // Формируем сообщение для отправки
        size_t messBSize = 0;
        char messBody[MESSAGE_LEN];
        recognizeMessage( buffer, &messBSize, &type, messBody );

        // Решаем что делать в зависимости от типа сообщения
        int kickFromChat = switchMessType( type, messBody, messBSize, login, &localParam );
        if (kickFromChat == 1) {
            return 0;
        }
    }
    close( newsockfd );
    rmUserFromList( usersList, login );
    *onlineCount = *onlineCount - 1;

    //free(login);

    printf( "Stopped talking!\n" );
    return 0;
}