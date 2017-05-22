#include "TCP_connection.h"
#include "Common.h"
#include "MessagesFormat.h"
#include "UsersModule.h"

// Отправляет историю сообщений пользователю
void sendHistory( int count, int sockfd, int lastMessId, struct CMessage* messBuffer ) {
    char message[MESSAGE_LEN];
    count = min( 50, count );
    for (int i = max( lastMessId - count, 0 ); i < lastMessId; ++i) {
        // В буфере уже сформированные сообщения для отправки
        size_t messSize = formMessage( message, 'h', messBuffer[i].mess, messBuffer[i].len );
        ssize_t n = write( sockfd, message, messSize );
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
    switch (type) {
        case 'i':
            // Регистрация/авторизация
            sendAnswer = 1; // Отвечать пользователю на его сообщение или нет
            type = 's';
            // Получаем логин и пароль
            char* login = (char*) malloc( 32 * sizeof( char ));
            char* password = (char*) malloc( 32 * sizeof( char ));
            bzero( login, 32 );
            bzero( password, 32 );
            int j = 0;
            int i = 0;
            for (i = 0; i < messBSize; ++i) {
                if (messBody[i] == '\n')
                    break;
                login[j++] = messBody[i];
            }
            j = 0;
            for (int k = i + 1/*Пропускаем \n*/; k < messBSize; ++k) {
                password[j++] = messBody[k];
            }
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
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '0';
                } else if (crStatus == 4) {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '4'; // Ошибка регистрации
                    kickFromChat = 1;
                } else if (crStatus == 5) {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '5'; // Ошибка доступа
                    kickFromChat = 1;
                }
            } else if (userStatus == 1) {
                sem_wait( semaphore );
                // Пытаемся войти
                int auStatus = authentication( usersList, login, password, newsockfd );
                sem_post( semaphore );
                if (auStatus == 0) {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '0';
                } else if (auStatus == 5) {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '5'; // kicked
                    kickFromChat = 1;
                } else {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '3'; // Ошибка аутентификации
                    kickFromChat = 1;
                }
            }
            strcpy( getLogin, login );
            if (messBody[0] == '0') {
                sem_wait( semaphore );
                (*onlineCount)++;
                sem_post( semaphore );
            }
            break;
        case 'r':
            // Обычное сообщение, добавляем к нему логин отправителя
            sendAnswer = 0;
            char* constMessBody = (char*) malloc( MESSAGE_LEN );
            char* lines[3];
            lines[0] = "1";
            lines[1] = getLogin;
            lines[2] = messBody;

            char messageBody[MESSAGE_LEN];
            size_t messBLen = 0;
            formMessageBody( messageBody, &messBLen, lines, 3 );
            size_t messSize = formMessage( constMessBody, 'r', messageBody, messBLen );

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
            sprintf( exitMess, "%s left the chat", login );
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
                kickRes = kick( usersList, atoi( messBody ), &kickedSock, &isOnline );
                if (isOnline == 1) {
                    size_t messSize = formMessage( message, 'k', "Sad...", messBSize );
                    ssize_t n = write( kickedSock, message, messSize );
                    if (n < 0) {
                        error( "ERROR writing to socket" );
                    }
                }
            }
            // Сообщаем root-у о результате
            type = 'm';
            if (kickRes == 0) {
                bzero( messBody, MESSAGE_LEN );
                strcpy( messBody, "Successfully kicked" );
            } else {
                bzero( messBody, MESSAGE_LEN );
                strcpy( messBody, "Can't kick" );
            }
            break;
        case 'h':
            sendHistory( atoi( messBody ), newsockfd, *lastMessId, messBuffer );
            break;
        default:
            sendAnswer = 1;
            printf( "Client is crazy! ->%d<-\n", type );
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