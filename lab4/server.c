#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define BUFLEN 81

int handler(int ssock) {
  char buf[BUFLEN];
  int msgLength;
  bzero(buf, BUFLEN);
  if ((msgLength = recv(ssock, buf, BUFLEN, 0)) < 0) {
    perror("Плохое получение дочерним процессом.");
    exit(1);
  }
  printf("SERVER: Socket для клиента - %d\n", ssock);
  printf("SERVER: Длина сообщения - %d\n", msgLength);
  printf("SERVER: Сообщение: %s\n\n", buf);
  return msgLength;
}

int main() {
  int msock;
  fd_set rfds;
  fd_set afds;
  int fd, nfds, ssock, length;
  struct sockaddr_in servAddr;

  if ((msock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Сервер не может открыть главный socket.");
    exit(1);
  }
  bzero((char *)&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = 0;
  if (bind(msock, &servAddr, sizeof(servAddr))) {
    perror("Связывание сервера неудачно.");
    exit(1);
  }

  length = sizeof(servAddr);
  if (getsockname(msock, &servAddr, &length)) {
    perror("Вызов getsockname неудачен.");
    exit(1);
  }
  printf("СЕРВЕР: номер порта - % d\n", ntohs(servAddr.sin_port));

  listen(msock, 5);
  nfds = getdtablesize();
  FD_ZERO(&afds);
  FD_SET(msock, &afds);
  while (1) {
    memcpy(&rfds, &afds, sizeof(rfds));
    if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <
        0) {
      printf("Ошибка селекта");
    }
    if (FD_ISSET(msock, &rfds)) {
      ssock = accept(msock, 0, 0);
      if (ssock < 0) {
        printf("Ошибка дочернего сокета");
      }
      FD_SET(ssock, &afds);
    }
    for (fd = 0; fd < nfds; fd++)
      if (fd != msock && FD_ISSET(fd, &rfds))
        if (handler(fd) == 0) {
          close(fd);
          FD_CLR(fd, &afds);
        }
  }
}
