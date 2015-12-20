#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "5555"
#define MAXDATASIZE 200
#define SIZE 3
#define GAME_START 1

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void get_picture(char morpion[SIZE][SIZE]) {
  printf("\n\n-- Game Board -- \n\n");
  printf("+---+---+---+\n");
  printf("| %c | %c | %c |\n", morpion[0][0], morpion[0][1], morpion[0][2]);
  printf("+---+---+---+\n");
  printf("| %c | %c | %c |\n", morpion[1][0], morpion[1][1], morpion[1][2]);
  printf("+---+---+---+\n");
  printf("| %c | %c | %c |\n", morpion[2][0], morpion[2][1], morpion[2][2]);
  printf("+---+---+---+\n\n");
}

void get_invite(int sockfd) {
  char buf[70];
  recv(sockfd, buf, sizeof buf, 0);
  printf(buf);
  int answer;
  scanf("%i", &answer);
  send(sockfd, &answer, sizeof(int), 0);
}


void play(int sockfd) {
  char buf[SIZE][SIZE];
  //int morpion[SIZE][SIZE];

  while (recv(sockfd, buf, MAXDATASIZE-1, 0) != 0) {
    system("clear");		/* Only works for UNIX, not portable */
    get_picture(buf);
    printf("Case à choisir (1-9): ");
    int to_send;
    scanf("%i", &to_send);
    send(sockfd, &to_send, 4, 0);
  }
}

int main(int argc, char *argv[]) {
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  /* Network to presentation of address */
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	    s, sizeof s);

  printf("Connecting to %s\n", s);
  freeaddrinfo(servinfo); // all done with this structure

  get_invite(sockfd);
  play(sockfd);
  close(sockfd);
  return 0;
}
