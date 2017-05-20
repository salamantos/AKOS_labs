all: server client

server: server.c MessagesFormat.c UsersModule.c MessagesReceiving.c SharedMemory.c
	gcc -o runServer server.c MessagesFormat.c UsersModule.c MessagesReceiving.c SharedMemory.c Common.c TCP_connection.c -Idg -pthread

client:
	gcc -o startClient client.c MessagesFormat.c -pthread
