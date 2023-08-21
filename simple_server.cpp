/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <netinet/in.h>
#include <pthread.h>
#include "http_server.hh"

using namespace std;

pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t count = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
vector<int> thread_arg;


//print error and exit
void error(char *msg) {
  perror(msg);
  exit(1);
}

//client is this w2
void *worker_function(void *arg) {
  /* read message from client - continuous */
  int newsockfd = *((int *) arg);
  char cbuffer[256];
  bzero(cbuffer, 256);
  while(1){
    int n = read(int(newsockfd), cbuffer, 255);
    if (n < 0) error("ERROR reading from socket");
    //printf("%s",cbuffer);

    //break on empty msg
    if(!strlen(cbuffer)){ printf("exiting on empty msg\n");break;}

    //printf("client message: %s \n", cbuffer);
    //bzero(cbuffer, 256); //clear buffer

    /* send reply to client - echo same */
    n = write(newsockfd, cbuffer, 255);//write is same as send with flag 0
    if (n < 0) error("ERROR writing to socket");
    //printf("sent");
    bzero(cbuffer, 256); //clear buffer
  }
  return 0;
}


//client is this w3
void *normal_function(void *arg) {
  /* read message from client - continuous */
  int newsockfd = *((int *) arg);
  cout<<newsockfd<<endl;
  char cbuffer[1256];
  bzero(cbuffer, 1256);
  //while(1){
    //cout<<"client: while init, before read\n";
    int n = read(int(newsockfd), cbuffer, 1255);
    if (n < 0) printf("ERROR reading from socket\n"); //only the thread to be killed, server should keep running
    //printf("%s\n",cbuffer);

    //break on empty msg
    if(!strlen(cbuffer)){ printf("exiting on empty msg\n");return 0;}
    //printf("client message: %s \n", cbuffer);
    //bzero(cbuffer, 1256); //clear buffer

    //cout<<"client: before handle_request\n";
    string buf = cbuffer;
    HTTP_Response* res = handle_request(buf);
    //cout<<"client: after handle_request\n ";//<<res->get_string(res)<<"\n";

    /* send reply to client - echo same */
    strcpy(cbuffer, (res->get_string(res)).c_str());
    //cout<<cbuffer;
    n = write(newsockfd, cbuffer, 1255);//write is same as send with flag 0
    //cout<<"client after write\n";
    if (n < 0) printf("ERROR writing to socket\n");
    bzero(cbuffer, 1256); //clear buffer
    close(newsockfd);//connection close
    //printf("client loop end\n");
  //}
  return 0;
}

//client is this w4
void *client_function(void *arg) {


  //need to change
  /* read message from client - continuous */
  label:
  pthread_mutex_lock(&lock);
  //cout<<"lock got"<<endl;
  //cout<<thread_arg.size()<<"arg"<<endl;
  while(thread_arg.size() == 0){
    //cout<<thread_arg.size()<<"before sleep"<<endl;
    pthread_cond_wait(&count, &lock);
    //cout<<thread_arg.size()<<"after sleep"<<endl;
  };
  //cout<<"entry\n";
  //while(thread_arg.size() == 0);
  int newsockfd = *thread_arg.begin();
  thread_arg.erase(thread_arg.begin());
  pthread_cond_signal(&full);
  pthread_mutex_unlock(&lock);
  //cout<<newsockfd<<"exit"<<endl;

  char cbuffer[1256];
  bzero(cbuffer, 1256);
  //while(1){
    //cout<<"client: while init, before read\n";
    int n = read(int(newsockfd), cbuffer, 1255);
    //cout<<cbuffer;
    if (n < 0) {
      printf("ERROR reading from socket\n");
      close(newsockfd);
      goto label;
    }
    if (n == 0) {
      printf("no connection, reading from socket\n");
      close(newsockfd);
      goto label;
    } //restart
    
    //printf("client message: %s \n", cbuffer);
    //bzero(cbuffer, 1256); //clear buffer

    //cout<<"client: before handle_request\n";
    string buf = cbuffer;
    HTTP_Response* res = handle_request(buf);
    cout<<"client: after handle_request\n "<<res->get_string(res)<<"\n";

    /* send reply to client - echo same */
    strcpy(cbuffer, (res->get_string(res)).c_str());
    //cout<<cbuffer;
    n = write(newsockfd, cbuffer, strlen(cbuffer));//write is same as send with flag 0
    //cout<<"client after write\n";
    if (n < 0) printf("ERROR writing to socket\n");
    bzero(cbuffer, 1256); //clear buffer
    close(newsockfd);//connection close
    //printf("client loop end\n");
  //}
  goto label;
  return 0;
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, PORT;
  struct sockaddr_in serv_addr;//socket


  //if no port provided ie no args
  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  PORT = atoi(argv[1]);//input port as arg;


  /* -----create socket----- */
  //system call to socket, af_inet is internet socket, 
  //sock_stream means connection based socket
  //last arg is protocol, 0 here, chooses protocol based on sock stream
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  //error if fd = -1
  if (sockfd < 0)
    error("ERROR opening socket\n");


  //assign struct vars with values - serv_addr object
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//inet_addr convert string to ipv4
  serv_addr.sin_port = htons(PORT);//


  /* bind socket to this port number on this machine */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding\n");


  /* listen for incoming connection requests */
  if (listen(sockfd, 5) < 0)//if listen queue full - it sends connection refused msg to client
    error("ERROR on listening\n");//no exit


  int i=0, limit=5, queue=5;
  cout<<"Enter thread pool limit:\n";
  cin>>limit;
  cout<<"Enter queue limit:\n";
  cin>>queue;
  cout<<"Waiting for requests...\n";
  /* accept a new request, create a newsockfd */
  pthread_t thread_id[limit];
  //init worker pool
  while(limit){
    pthread_create(&thread_id[i], NULL, client_function, NULL);
    i++;limit--;
  }
  limit=i;
  
  while(1){
    //cout<<"server: init\n"<<i<<"\n";
    struct sockaddr_in cli_addr;//socket
    socklen_t clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    cout<<"server: after accept\n";
    if (newsockfd < 0) {printf("ERROR on accept\n");continue;}//ntoa - ascii
    else printf("Connected to client on address %s, post %d \n", inet_ntoa(cli_addr.sin_addr),cli_addr.sin_port);

    //mutex lock
    //need to change
    pthread_mutex_lock(&lock);
    while(thread_arg.size() >= queue){
      //cout<<thread_arg.size()<<"before sleep"<<endl;
      pthread_cond_wait(&full, &lock);
      //cout<<thread_arg.size()<<"queue full"<<endl;
    };
    thread_arg.push_back(newsockfd);
    pthread_cond_signal(&count);
    pthread_mutex_unlock(&lock);
    //cout<<"unlock main\n";
    cout<<thread_arg[thread_arg.size()-1]<<"afterlock"<<endl;
    
  }
   
  pthread_join(*thread_id, NULL);
  close(sockfd);
  return 0;
}
