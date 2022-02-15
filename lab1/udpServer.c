#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#include <netdb.h>

#include <errno.h>

#define BUFLEN 81
#include <stdlib.h>

int main() {
  int sockMain, msgLength;
  socklen_t addrLength;
  struct sockaddr_in servAddr, clientAddr;
  char buf[BUFLEN];
  if ((sockMain = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Сервер не может открыть socket для UDP.");
    exit(1);
  }

  bzero((char *)&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = 0;

  if (bind(sockMain,(struct sockaddr *) &servAddr, sizeof(servAddr))) {
    perror("Вызов bind от сервера неудачен.");
    exit(1);
  }
  addrLength = sizeof(servAddr);
  if (getsockname(sockMain,(struct sockaddr *) &servAddr, &addrLength)) {
    perror("Вызов getsockname неудачен.");
    exit(1);
  }
  printf("SERVER: Номер порта - %d\n", ntohs(servAddr.sin_port));
  for (;;) {
    addrLength = sizeof(clientAddr);
    bzero(buf, BUFLEN);
    if ((msgLength = recvfrom(sockMain, buf, BUFLEN, 0,(struct sockaddr *) &clientAddr,
                              &addrLength)) < 0) {
      perror("Плохой socket клиента.");
      exit(1);
    }

    if (sendto(sockMain, buf, msgLength, 0, (struct sockaddr *) &clientAddr, addrLength) < 0 ){
      perror("Проблема с sendto.\n");
      exit(1);
    }
    
    printf("SERVER: IP-адрес клиента: %s\n",
           inet_ntoa(clientAddr.sin_addr));
    printf("SERVER: Порт клиента: %d\n",
           ntohs(clientAddr.sin_port));
    printf("SERVER: Длина сообщения %d\n", msgLength);
    printf("SERVER: Сообщение: %s\n\n", buf);
  }
}
