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
#define BACKLOG 3 		/* Pending connections the queue will hold */

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

void playMorpion(int new_fd) {
  char *display;
  int morpion[3][3] = {0};
  display = "asdf";
  char *otro = "qwer";
  display = strcat(display, otro);

  //size_t taille = sizeof display;

  //send(new_fd, &taille, sizeof(size_t), 0);
  send(new_fd, display, sizeof display, 0);
}

int main(void) {

  struct addrinfo *goodies = getGoodies();
  int sockfd = prepareSocket(goodies);
  struct sockaddr_in6 ext_addr;
  socklen_t sin_size;

  printf("Server waiting for connection...\n");


  for (; ;) {
    int new_fd; 			/* New file descriptor */
    sin_size = sizeof ext_addr;
    if ((new_fd = accept(sockfd, (struct sockaddr *)&ext_addr, &sin_size)) == -1) {
      perror("Error extracting connection request");
      exit(EXIT_FAILURE);
    }

    if (!fork()) {
      close(sockfd); 		/* Close listener, dont need it */
      playMorpion(new_fd);
      close(new_fd);
      exit(EXIT_SUCCESS);
    }
    close(new_fd); 		/* Already forked */

  }


  return 0;
}
