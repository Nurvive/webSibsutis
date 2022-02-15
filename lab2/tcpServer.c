#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
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

void reaper(int sig) {
  int status;
  while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
    ;
}

int main() {
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

  signal(SIGCHLD, reaper);
  while (1) {
    if ((sockClient = accept(sockMain, 0, 0)) < 0) {
      perror("Неверный socket для клиента.");
      exit(1);
    }
    switch (fork()) {
    case (0):
      close(sockMain);
      char buf[BUFLEN];
      int msgLength;
      bzero(buf, BUFLEN);
      if ((msgLength = recv(sockClient, buf, BUFLEN, 0)) < 0) {
        perror("Плохое получение дочерним процессом.");
        exit(1);
      }
      printf("SERVER: Socket для клиента - %d\n", sockClient);
      printf("SERVER: Длина сообщения - %d\n", msgLength);
      printf("SERVER: Сообщение: %s\n\n", buf);
      close(sockClient);
      exit(0);
    default:
      close(sockClient);
      break;
    case -1:
      printf("Ошибка\n");
    }
  }
}
