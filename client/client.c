#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<pthread.h>

#define MAX_LINE 100
#define SERV_PORT 8000

#define handle_error(msg) \
  do { \
     perror(msg);\
     exit(EXIT_FAILURE);\
   }while(0)

void stop(int signo);
void *handleRecv(void *arg);
void *handleWrite(void *arg);

int sockfd;

int main(int argc,char *argv[])
{
  struct sockaddr_in server;
  //char buf[MAX_LINE];
  //int n;
  pthread_t pid_r = -1;
  pthread_t pid_w = -1;
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  signal(SIGINT,stop);
  memset(&server,0,sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(SERV_PORT);
  if(-1 == connect(sockfd,(struct sockaddr *)&server,sizeof(server)))
  {

    handle_error("connet");
  }
  if(pthread_create(&pid_r,NULL,handleRecv,NULL) < -1)
  {
     handle_error("pthread_create recv failed..\n");
  }
  if(pthread_create(&pid_w,NULL,handleWrite,NULL) < -1)
  {
     handle_error("pthread_create write failed..\n");
  }
  for(;;);
  printf("finished ....\n");
  close(sockfd);
  return 0;
}

void stop(int signo)
{
  printf("stop...\n");
  close(sockfd);
  exit(0);
}

void *handleWrite(void *arg)
{
  char buf[MAX_LINE] = {0};

  while(1){
    memset(buf,0,MAX_LINE);
    //printf("please input msg to server: \n");
    fgets(buf,MAX_LINE,stdin);
    if(!strncmp(buf,"closed",strlen("closed"))){
       break;
    }
    write(sockfd,buf,strlen(buf));
    printf("send:%s",buf);
    memset(buf,0,MAX_LINE);
  }
}

void *handleRecv(void *arg)
{
  char buf[MAX_LINE] = {0};
  int num;
  for(;;)
  {
    num = read(sockfd,buf,MAX_LINE);
    if(num  == -1 || num == 0)
    {
      handle_error("read error..\n");   
      exit(0); 
    }
    else
    {
      printf("recv:%s",buf);
      //fflush(stdout);
      if(strncmp(buf,"server:close",MAX_LINE) == 0)
      {
        exit(0);
      }
    }
    memset(buf,0,MAX_LINE); 
  }

}

void handleMsg(const char *buf)
{


}
