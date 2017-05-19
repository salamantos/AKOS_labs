#include "Common.h"
#include "TCP_connection.h"

void* newThread( void* param ) {
    connectNewUser( (struct CThreadParam*) param );
}