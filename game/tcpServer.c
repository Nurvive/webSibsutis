#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define BUFLEN 64
#define SEM_ID 2001
#define SHM_ID 2002
#define MSG_TYPE_EMPTY 0
#define MSG_TYPE_FINISH 2
#define PERMS 0666
#define PORT 4444

unsigned int players = 1;

typedef struct {
  int type;
  char string[BUFLEN];
} message_t;

int decision(char first[BUFLEN], char second[BUFLEN]) {
  if (strcmp(first, second) == 0)
    return 0;
  if (!strcmp(first, "1") && !strcmp(second, "2"))
    return 1;
  if (!strcmp(first, "1") && !strcmp(second, "3"))
    return 2;
  if (!strcmp(first, "2") && !strcmp(second, "3"))
    return 1;
  if (!strcmp(first, "2") && !strcmp(second, "1"))
    return 2;
  if (!strcmp(first, "3") && !strcmp(second, "1"))
    return 1;
  if (!strcmp(first, "3") && !strcmp(second, "2"))
    return 2;
  return -1;
}

void game(int sockClient, unsigned int id) {
  char recvBuf[BUFLEN];
  char secondPlayer[BUFLEN];
  char sendBuf[BUFLEN * 2];
  bzero(recvBuf, BUFLEN);
  bzero(secondPlayer, BUFLEN);
  bzero(sendBuf, BUFLEN);
  int semid;
  int shmid;
  message_t *msg_p;
  if ((semid = semget(SEM_ID, 1, PERMS | IPC_CREAT)) < 0) {
    printf("СЕРВЕР: can not get semaphore, %d\n", errno == EACCES);
    printf("%d, %d\n", errno == EEXIST, errno == ENOENT);
    exit(1);
  }
  if ((shmid = shmget(SHM_ID, sizeof(message_t), PERMS | IPC_CREAT)) < 0) {
    printf("СЕРВЕР: can not get shared memory segment, %d\n", errno);
    exit(1);
  }
  if ((msg_p = (message_t *)shmat(shmid, 0, 0)) == NULL) {
    printf("СЕРВЕР: shared memory attach error\n");
    exit(1);
  }
  semctl(semid, 0, SETVAL, 0);
  msg_p->type = MSG_TYPE_EMPTY;
  sprintf(sendBuf, "Ты игрок %d", id);
  send(sockClient, sendBuf, BUFLEN, 0);
  bzero(sendBuf, BUFLEN);
  while (1) {
    if ((recv(sockClient, recvBuf, BUFLEN, 0)) < 0) {
      perror("Плохое получение дочерним процессом.\n");
      exit(1);
    }
    if (strcmp(recvBuf, "-1") == -1) {
      printf("Конец игры для %d\n", id);
      break;
    } else {
      if (id == 1) {
        while (semctl(semid, 0, GETVAL, 0) || msg_p->type == MSG_TYPE_EMPTY)
          ;
        semctl(semid, 0, SETVAL, 1);
        strncpy(secondPlayer, msg_p->string, BUFLEN);
        bzero(msg_p->string, BUFLEN);
        int result = decision(recvBuf, secondPlayer);
        if(result == 0) {
          sprintf(sendBuf, "Ничья!");
        } else if (result == 1 || result == 2){
          sprintf(sendBuf, "Выиграл игрок %d!", result);
        } else if(result == -1) {
          sprintf(sendBuf, "Случилось что-то странное.");
        }
        strncpy(msg_p->string, sendBuf, BUFLEN);
        msg_p->type = MSG_TYPE_EMPTY;
        semctl(semid, 0, SETVAL, 0);
      } else {
        semctl(semid, 0, SETVAL, 1);
        strncpy(msg_p->string, recvBuf, BUFLEN);
        msg_p->type = MSG_TYPE_FINISH;
        semctl(semid, 0, SETVAL, 0);
        while (semctl(semid, 0, GETVAL, 0) || msg_p->type != MSG_TYPE_EMPTY)
          ;
        semctl(semid, 0, SETVAL, 1);
        strncpy(sendBuf, msg_p->string, BUFLEN);
        bzero(msg_p->string, BUFLEN);
        msg_p->type = MSG_TYPE_EMPTY;
        semctl(semid, 0, SETVAL, 0);
      }
      // sprintf(sendBuf, "Ты выбрал - %s", recvBuf);
      send(sockClient, sendBuf, BUFLEN, 0);
      bzero(recvBuf, BUFLEN);
      bzero(sendBuf, BUFLEN);
      bzero(secondPlayer, BUFLEN);
    }
  }
  shmdt(msg_p);
}

int main() {
  int sockfd;
  struct sockaddr_in serverAddr;

  int newSocket;
  struct sockaddr_in newAddr;

  socklen_t addr_size;

  char buffer[1024];
  bzero(buffer, 1024);
  pid_t childpid;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("СЕРВЕР: не могу открыть сокет.\n");
    exit(1);
  }

  memset(&serverAddr, '\0', sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("СЕРВЕР: не могу забиндить.\n");
    exit(1);
  }

  if (listen(sockfd, 2) == 0) {
    printf("СЕРВЕР: слушаю.\n");
  } else {
    printf("СЕРВЕР: не слушаю.\n");
  }

  while (1) {
    newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
    if (newSocket < 0) {
      exit(1);
    }
    printf("СЕРВЕР: соединился с %s:%d\n", inet_ntoa(newAddr.sin_addr),
           ntohs(newAddr.sin_port));
    if ((childpid = fork()) == 0) {
      close(sockfd);
      game(newSocket, players);
    } else {
      players += 1;
    }
  }
  close(newSocket);
  return 0;
}
