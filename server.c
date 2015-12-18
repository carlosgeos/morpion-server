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
    perror("Failed creating socket"); /* and print last error encountered */
    exit(EXIT_FAILURE);
  }

  int reuse_addr_to_true = 1;
  if (setsockopt(sockfd, SOL_SOCKET,
  		 SO_REUSEADDR, &reuse_addr_to_true, sizeof(int)) == -1) {
    perror("Failed setting reuse address option for the socket");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, goodies -> ai_addr, goodies -> ai_addrlen) == -1) {
    close(sockfd);
    perror("Failed to assign addr to socket file descriptor");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(goodies); 	/* No need for this anymore */

  if (listen(sockfd, BACKLOG)) {
    perror("Could not mark the socket referenced by sockfd as passive");
    exit(EXIT_FAILURE);
  }

  return sockfd;

}

void sendPicture(int morpion[SIZE][SIZE], int new_fd) {
  int rows;
  int columns;
  char display[50] = "\n-- Game -- \n";
  for (rows = 0; rows < SIZE; ++rows) {
    strcat(display, "[");
    for (columns = 0; columns < SIZE; ++columns) {
      if (morpion[rows][columns] == 1) {
	strcat(display, " X");
      } else if (morpion[rows][columns] == -1) {
	strcat(display, " 0");
      } else {
	strcat(display, " .");
      }
    }
    strcat(display, " ]\n");
  }

  send(new_fd, display, sizeof display, 0);
}

void playMorpion(int new_fd) {
  int morpion[SIZE][SIZE] = {0};
  morpion[1][1] = 1;
  morpion[2][2] = -1;
  sendPicture(morpion, new_fd);

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
      perror("Error extracting connection request");
      exit(EXIT_FAILURE);
    }

    char from[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(guest.sin6_addr), from, INET6_ADDRSTRLEN);
    printf("Got connection from: %s\n", from);

    if (!fork()) {
      close(sockfd); 		/* Close listener, dont need it */
      char invite[50] = "Envoyer le chiffre 1 pour jouer, le chiffre 2 pour se déconnecter";
      send(new_fd, invite, sizeof invite, 0);
      char buf[100];
      recv(new_fd, buf, sizeof buf, 0);
      playMorpion(new_fd);
      close(new_fd);
      exit(EXIT_SUCCESS);
    }
    close(new_fd); 		/* Already forked */

  }


  return 0;
}
