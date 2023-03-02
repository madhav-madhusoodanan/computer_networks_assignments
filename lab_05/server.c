#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32

char* STRING_HANDLER = NULL;

char* read(FILE *db, char *key)
{
    fseek(db, 0, SEEK_SET);
    char buffer[2 * BUFFERSIZE + 1];
    char keyLocal[BUFFERSIZE];
    char value[BUFFERSIZE];
    bzero(value, BUFFERSIZE);

    int found = 0;
    while (!found)
    {
        fscanf(db, "%s");
        sscanf(buffer, "%s %s", keyLocal, value);
        if (strcmp(key, keyLocal) == 0) found = 1;
    }

    return value;
}

int write(FILE *db, char *key, char *value) {
    if(!strlen(value) || !strlen(key)) return 0;
    fseek(db, 0, SEEK_END);
    fprintf(db, "%s %s", key, value);
    return 1;
}

int delete(FILE *db, char *key) {
    FILE* fp = fopen("_temp", "w");

    // COPY VALUES INTO TEMP
    fseek(db, 0, SEEK_SET);
    char buffer[2 * BUFFERSIZE + 1];

    // CHECK IF THE STRING READ IS EQUAL TO KEY
    // DO NOT COPY THAT LINE
    char keyLocal[BUFFERSIZE];
    char value[BUFFERSIZE];
    bzero(value, BUFFERSIZE);

    int found = 0;
    while (!found)
    {
        fscanf(db, "%s");
        sscanf(buffer, "%s %s", keyLocal, value);
        if(!strlen(value) || !strlen(key)) continue;
        fprintf(fp, "%s %s", keyLocal, value);
    }

    // RENAME FILE BACK TO DB
    fclose(db);
    remove("db");
    rename("_temp", "db");
    db = fp;
    return 0;
}

char* handler(FILE* db, char* string){
    char op[BUFFERSIZE];
    char key[BUFFERSIZE];
    char value[BUFFERSIZE];

    bzero(value, BUFFERSIZE);
    sscanf(string, "%s %s %s", op, key, value);

    if(strcmp(op, "get") == 0) strcpy(value, read(db, key));
    else if(strcmp(op, "write") == 0) write(db, key, value);
    else if(strcmp(op, "delete") == 0) delete(db, key);

    return value;
}

int main()
{
    FILE *db = fopen("db", "w+");

    /*CREATE A TCP SOCKET*/
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        printf("Error while server socket creation");
        exit(0);
    }
    printf("Server Socket Created\n");

    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddress, clientAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");
    int temp = bind(serverSocket, (struct sockaddr *)&serverAddress,
                    sizeof(serverAddress));
    if (temp < 0)
    {
        printf("Error while binding\n");
        exit(0);
    }
    printf("Binding successful\n");
    int temp1 = listen(serverSocket, MAXPENDING);
    if (temp1 < 0)
    {
        printf("Error in listen");
        exit(0);
    }
    printf("Now Listening\n");
    char msg[BUFFERSIZE];
    bzero(msg, BUFFERSIZE);
    int clientLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength);
    if (clientLength < 0)
    {
        printf("Error in client socket");
        exit(0);
    }
    printf("Handling Client %s\n", inet_ntoa(clientAddress.sin_addr));
    int temp2 = recv(clientSocket, msg, BUFFERSIZE, 0);
    if (temp2 < 0)
    {
        printf("problem in temp 2");
        exit(0);
    }
    printf("%s\n", msg);
    printf("ENTER MESSAGE FOR CLIENT\n");
    scanf(" %s", msg);
    int bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
    {
        printf("Error while sending message to client");
        exit(0);
    }
    close(serverSocket);
    close(clientSocket);
    fclose(db);
}