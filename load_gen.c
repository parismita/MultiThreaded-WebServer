/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>

int time_up;


// user info struct
struct user_info {
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  float total_rtt;
};

// error handling function
void error(char *msg) {
  perror(msg);
  //exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1) {
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg) {
  /* get user info */
  struct user_info *info = (struct user_info *)arg;

  int sockfd, n;
  char buffer[1256];
  struct timeval start, end;

  struct sockaddr_in serv_addr;
  struct hostent *server = gethostbyname(info->hostname);

  while (1) {
    /* start timer */
    gettimeofday(&start, NULL);

    /* TODO: create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {error("ERROR opening socket");usleep(info->think_time*1000000);
      continue;
    }

    /* TODO: set server attrs */
    /*bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(8000);*/
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//inet_addr convert string to ipv4
    serv_addr.sin_port = htons(info->portno);//

    /* TODO: connect to server */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    { 
      close(sockfd);error("ERROR connecting");usleep(info->think_time*1000000);
      continue;
    }

    /* TODO: send message to server */
    bzero(buffer, 1256);
    strcpy(buffer, "GET / HTTP/1.1\n");
    //printf("check3\n");
    n = write(sockfd, buffer, strlen(buffer));
    if(n<0) {
      close(sockfd);error("ERROR writing");usleep(info->think_time*1000000);
      continue;
      }
    //printf("check4%d\n", n);

    /* TODO: read reply from server */
    bzero(buffer, 1256);
    n = read(sockfd, buffer, 1255);
    if(n<0) {
      close(sockfd);error("ERROR reading");usleep(info->think_time*1000000);
      continue;
    }
    //printf("check5%d\n", n);

    /* TODO: close socket */
    close(sockfd);
    //printf("check6\n");

    /* end timer */
    gettimeofday(&end, NULL);

    /* if time up, break */
    if (time_up)
      break;

    /* TODO: update user metrics */
    info->total_count++;
    info->total_rtt+=time_diff(&end, &start);

    //gettimeofday(&start, NULL);
    /* TODO: sleep for think time */
    //printf("before think%f%f\n", info->think_time, info->total_rtt);
    //struct timeval req = {(int)info->think_time,(info->think_time-(int)info->think_time)*1000000};
    //nanosleep(&req, NULL);
    //fd_set rfds;
    //int retval = select(1, &rfds, NULL, NULL, &req);//return fds after req time - instead of sleep it waits
    usleep(info->think_time*1000000);
    //gettimeofday(&end, NULL);
    //printf("after think%f %f %d %d\n", info->think_time, time_diff(&end, &start), req.tv_sec, req.tv_nsec);
    //printf("after think%f %f\n", info->think_time, time_diff(&end, &start));

  }

  /* exit thread */
  //fprintf(log_file, "User #%d finished\n", info->id);
  //fflush(log_file);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int user_count, portno, test_duration;
  float think_time;
  char *hostname;

  if (argc != 6) {
    fprintf(stderr,
            "Usage: %s <hostname> <server port> <number of concurrent users> "
            "<think time (in s)> <test duration (in s)>\n",
            argv[0]);
    exit(0);
  }

  hostname = argv[1];
  portno = atoi(argv[2]);
  user_count = atoi(argv[3]);
  think_time = atof(argv[4]);
  test_duration = atoi(argv[5]);

  /*printf("Hostname: %s\n", hostname);
  printf("Port: %d\n", portno);
  printf("User Count: %d\n", user_count);
  printf("Think Time: %f s\n", think_time);
  printf("Test Duration: %d s\n", test_duration);*/

  /* open log file */
  FILE *file;
  file = fopen("load.txt", "a+");

  pthread_t threads[user_count];
  struct user_info info[user_count];
  struct timeval start, end;

  /* start timer */
  gettimeofday(&start, NULL);
  //printf("check1\n");
  time_up = 0;
  for (int i = 0; i < user_count; ++i) {
    /* TODO: initialize user info */
    info->think_time=think_time;
    info->id=i;
    info->hostname=hostname;
    info->portno=portno;
    info->total_count=0;
    info->total_rtt=0;
    //printf("check7%f%f\n", info->think_time, think_time);
    //printf("check2%d\n",i);
    /* TODO: create user thread */
    if(pthread_create(&threads[i], NULL, user_function, info)) printf("error\n");
    //fprintf(log_file, "Created thread %d\n", i);
  }

  /* TODO: wait for test duration */
  sleep(test_duration);

  //fprintf(log_file, "Woke up\n");

  /* end timer */
  time_up = 1;
  gettimeofday(&end, NULL);

  /* TODO: wait for all threads to finish */
  for(int i=0;i<user_count;i++)
    {
      pthread_join(threads[i], NULL);
    }
  //pthread_exit(NULL);
  /* TODO: print results */
  info->total_rtt=info->total_rtt/info->total_count;
  fprintf(file, "%d %d %f\n",user_count, info->total_count/test_duration, info->total_rtt);

  /* close log file */
  fclose(file);

  return 0;
}
