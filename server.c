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
#define PLAYER 1
#define MACHINE 0

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
    return 0;
  }
  return 1;
}

int mark_case(char morpion[SIZE][SIZE], int choice, int player) {
  int x;
  int y;
  if (!choice%SIZE) {
    x = (choice/SIZE) - 1; y = SIZE - 1;
  } else {
    x = choice/SIZE; y = (choice%SIZE) - 1;
  }

  if (libre(morpion, x, y)) {
    if (player) {
      morpion[x][y] = 'X';
    } else {
      morpion[x][y] = 'O';
    }
    return 1;
  }
  return 0;
}

void choose_case(char morpion[SIZE][SIZE]) {
  int chosen = 0;
  while (!chosen) {
    int choice = rand() % 9 + 1;
    printf("choice machine: %i\n", choice);
    if (mark_case(morpion, choice, MACHINE)) chosen = 1;
  }
}

/* TODO: End game condition not working */

/* int game_won(char morpion[SIZE][SIZE]) { */
/*   if((morpion[0][0] == morpion[1][1] && */
/*       morpion[0][0] == morpion[2][2]) || */
/*      (morpion[0][2] == morpion[1][1] && */
/*       morpion[0][2] == morpion[2][0])) */
/*     return 1; */
/*   else */
/*     for(int row = 0; row <= 2; row ++) */
/*       if((morpion[row][0] == morpion[row][1] && */
/* 	  morpion[row][0] == morpion[row][2])|| */
/* 	 (morpion[0][row] == morpion[1][row] && */
/* 	  morpion[0][row] == morpion[2][row])) */
/* 	return 1; */
/*   return 0; */
/* } */


void play_morpion(int new_fd) {
  int times;
  //int winner = 0;
  char morpion[SIZE][SIZE] = {{'1', '2', '3'},
			     {'4', '5', '6'},
			     {'7', '8', '9'}};
  int choice;
  for (times = 0; times < SIZE*SIZE; ++times) {

    /* if (game_won(morpion)) */
    /*   done = 1; winner = PLAYER; out(winner); */
    choose_case(morpion);
    send(new_fd, morpion, sizeof morpion, 0);
    /* if (game_won(morpion)) */
    /*   done = 1; winner = MACHINE; out(winner); */
    recv(new_fd, &choice, sizeof choice, 0);

    if (choice > 0 && choice < 10) {
      mark_case(morpion, choice, PLAYER);
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
