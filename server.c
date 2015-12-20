#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "5555"
#define BACKLOG 5 		/* Pending connections the queue will hold */
#define SIZE 3


const int GAME_START = 1;

struct addrinfo *getGoodies(void) {
  struct addrinfo *goodies;
  struct addrinfo suggestions;		  /* Main data structure */
  memset(&suggestions, 0, sizeof(suggestions));
  suggestions.ai_family = AF_INET6;	 /* IPv4 not IPv6 */
  suggestions.ai_protocol = IPPROTO_TCP; /* TCP no raw or UDP */
  suggestions.ai_socktype = SOCK_STREAM; /* TCP connection oriented */

  int ecode;
  if((ecode = getaddrinfo(NULL, PORT, &suggestions, &goodies)) != 0) {
    printf("Failed getting address information: %s\n" , gai_strerror(ecode));
  } /* status should be 0, not -1 */

  return goodies;
}

int prepareSocket(struct addrinfo* goodies) {
  int sockfd;
  if ((sockfd = socket(goodies -> ai_family,
		       goodies -> ai_socktype,
		       goodies -> ai_protocol)) == -1) {
    perror("server - Failed creating socket"); /* and print last error encountered */
    exit(EXIT_FAILURE);
  }

  int reuse_addr_to_true = 1;
  if (setsockopt(sockfd, SOL_SOCKET,
  		 SO_REUSEADDR, &reuse_addr_to_true, sizeof(int)) == -1) {
    perror("server - Failed setting reuse address option for the socket");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, goodies -> ai_addr, goodies -> ai_addrlen) == -1) {
    close(sockfd);
    perror("server - Failed to assign addr to socket file descriptor");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(goodies); 	/* No need for this anymore */

  if (listen(sockfd, BACKLOG)) {
    perror("server - Could not mark the socket referenced by sockfd as passive");
    exit(EXIT_FAILURE);
  }

  return sockfd;

}

int libre(char morpion[SIZE][SIZE], int x, int y) {
  if (morpion[x][y] == 'X' || morpion[x][y] == 'O') {
    printf("busy!!!");
    return 0;
  }
  return 1;
}

void mark_case(char morpion[SIZE][SIZE], int choice) {
  int x;
  int y;
  if (!choice%SIZE) {
    x = (choice/SIZE) - 1; y = SIZE - 1;
  } else {
    x = choice/SIZE; y = (choice%SIZE) - 1;
  }

  if (libre(morpion, x, y)) {
    morpion[x][y] = 'X';
  }
}

void play_morpion(int new_fd) {
  int done = 0;
  char morpion[SIZE][SIZE] = {{'1', '2', '3'},
			     {'4', '5', '6'},
			     {'7', '8', '9'}};
  int choice;
  while(!done) {
    send(new_fd, morpion, sizeof morpion, 0);
    recv(new_fd, &choice, sizeof choice, 0);

    if (choice > 0 && choice < 10) {
      mark_case(morpion, choice);

    }
  }
}

int invite(const int new_fd) {
  char invite[70] = "\nEnvoyer le chiffre 1 pour jouer, le chiffre 2 pour se déconnecter: ";
  int answer = 0;
  send(new_fd, invite, sizeof invite, 0);
  int say;
  recv(new_fd, &say, sizeof say, 0);
  if (say == 1) {
    answer = say;
  }
  return answer;
}


int main(void) {

  struct addrinfo *goodies = getGoodies();
  int sockfd = prepareSocket(goodies); /* Listener */
  struct sockaddr_in6 guest;
  socklen_t sin_size;

  printf("Server waiting for connection...\n");

  while (1) {
    int new_fd; 			/* New file descriptor, where stuff happens */
    sin_size = sizeof guest;
    if ((new_fd = accept(sockfd, (struct sockaddr *)&guest, &sin_size)) == -1) {
      perror("server - Error extracting connection request");
      exit(EXIT_FAILURE);
    }

    char from[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(guest.sin6_addr), from, INET6_ADDRSTRLEN);
    printf("server - Got connection from: %s\n", from);

    if (!fork()) {
      close(sockfd); 		/* Close listener, dont need it */
      if (invite(new_fd)) play_morpion(new_fd);
      close(new_fd);
      exit(EXIT_SUCCESS);
    }
    close(new_fd); 		/* Already forked */

  }


  return 0;
}
