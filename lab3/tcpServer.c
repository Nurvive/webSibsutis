#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define BUFLEN 81

struct {
  pthread_mutex_t st_mutex;
} GLOBAL;

void *handler(void *ssock) {
  pthread_mutex_lock(&GLOBAL.st_mutex);
  char buf[BUFLEN];
  int msgLength;
  bzero(buf, BUFLEN);
  if ((msgLength = recv((int)ssock, buf, BUFLEN, 0)) < 0) {
    perror("Плохое получение дочерним процессом.");
    exit(1);
  }
  printf("SERVER: Socket для клиента - %d\n", (int)ssock);
  printf("SERVER: Длина сообщения - %d\n", msgLength);
  printf("SERVER: Сообщение: %s\n\n", buf);
  close((int)ssock);
  pthread_mutex_unlock(&GLOBAL.st_mutex);
  return 0;
}

int main() {
  pthread_t th;
  pthread_attr_t ta;
  int sockMain, sockClient, length;
  struct sockaddr_in servAddr;

  if ((sockMain = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Сервер не может открыть главный socket.");
    exit(1);
  }
  bzero((char *)&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = 0;
  if (bind(sockMain, &servAddr, sizeof(servAddr))) {
    perror("Связывание сервера неудачно.");
    exit(1);
  }

  length = sizeof(servAddr);
  if (getsockname(sockMain, &servAddr, &length)) {
    perror("Вызов getsockname неудачен.");
    exit(1);
  }
  printf("СЕРВЕР: номер порта - % d\n", ntohs(servAddr.sin_port));

  listen(sockMain, 5);

  pthread_attr_init(&ta);
  pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
  pthread_mutex_init(&GLOBAL.st_mutex, 0);

  while (1) {
    sockClient = accept(sockMain, 0, 0);
    if (sockClient < 0) {
      printf("Неверный socket для клиента");
      exit(1);
    }
    if (pthread_create(&th, &ta, handler, (void *)sockClient) < 0) {
      printf("Ошибка потока\n");
    }
  }
}
