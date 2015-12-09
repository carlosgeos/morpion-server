
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


struct addrinfo * getGoodies(void) {
  struct addrinfo *goodies;
  struct addrinfo suggestions;		  /* Main data structure */
  memset(&suggestions, 0, sizeof(suggestions));
  suggestions.ai_family = AF_UNSPEC; /* IP4 or IP6 */
  suggestions.ai_protocol = IPPROTO_TCP; /* TCP no raw or UDP */
  suggestions.ai_socktype = SOCK_STREAM; /* TCP connection oriented */
  //suggestions.ai_flags = AI_PASSIVE;

  /* Null without ai passive gives localhost afterwards, for
     applications that intent to communicate with peers on same machine */

  int status = getaddrinfo(NULL, PORT, &suggestions, &goodies); /* status should be 0, not -1 */
  //char ipstr[INET6_ADDRSTRLEN];

  /* for (gp = goodies; gp != NULL; gp = gp -> ai_next) { */
  /*   void *addr; */
  /*   char *ipver; */
  /*   int sockett; */

  /*   // get the pointer to the address itself, */
  /*   // different fields in IPv4 and IPv6: */
  /*   if (gp->ai_family == AF_INET) { // IPv4 */
  /*     struct sockaddr_in *ipv4 = (struct sockaddr_in *)gp->ai_addr; */
  /*     addr = &(ipv4->sin_addr); */
  /*     ipver = "IPv4"; */
  /*     sockett = gp -> ai_socktype; */

  /*   } else { // IPv6 */
  /*     struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)gp->ai_addr; */
  /*     addr = &(ipv6->sin6_addr); */
  /*     ipver = "IPv6"; */
  /*     sockett = gp -> ai_socktype;; */

  /*   } */
  /*   // convert the IP to a string and print it: */
  /*   inet_ntop(gp->ai_family, addr, ipstr, sizeof ipstr); */
  /*   printf(" %s: %s\n", ipver, ipstr); */
  /*   printf("Socket type: %i\n", sockett); */
  /* } */

  return gp;

}

int main(void) {

  struct addrinfo *pointer = getGoodies();
  int sockfd;

  //struct sockaddr_storage connector_addr; /* Can be IPv4 or IPv6, like
  //					     sockaddr_in or
  //sockaddr_in6 */



  freeaddrinfo(pointer);


  return 0;
}
