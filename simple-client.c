/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

void error(char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockfd, PORT, n;
  struct sockaddr_in serv_addr;
  struct hostent *SERVER;

  char cbuffer[256];

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }
  PORT = atoi(argv[2]);
  SERVER = gethostbyname(argv[1]);


  /* create socket, get sockfd handle */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");


  if (SERVER == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }


  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)SERVER->h_addr, (char *)&serv_addr.sin_addr.s_addr, SERVER->h_length);
  serv_addr.sin_port = htons(PORT);


  /* connect to server */
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  while(1){
    /* ask user for input */
    printf("Please enter the message: ");
    bzero(cbuffer, 256);
    fgets(cbuffer, 255, stdin);

    if(!strcmp(cbuffer, "exit\n")) {printf("no input, exiting\n"); break;}


    /* send user message to server */
    n = write(sockfd, cbuffer, strlen(cbuffer));
    if (n < 0) error("ERROR writing to socket\n");
    bzero(cbuffer, 256);

    /* read reply from server */
    n = read(sockfd, cbuffer, 255);
    if (n < 0) error("ERROR reading from socket\n");
    printf("Server response: %s\n", cbuffer);
    bzero(cbuffer, 256);
  }
  close(sockfd);

  return 0;
}
