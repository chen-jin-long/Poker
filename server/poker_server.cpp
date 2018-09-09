#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "../game/poker_game.h"
#define MAX_LINE 100
#define SERV_PORT 8000

int main()
{
    struct sockaddr_in server,client;
    int listenfd,connectfd, client_addr_len,n;
    socklen_t clientaddr_len;
    //char buf[MAX_LINE];
    //POKER_DESK *pdesk = NULL;
    POKER_ROOM proom[MAX_POKER_ROOM]; 
    setupPokerRoom(proom);
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERV_PORT);

    bind(listenfd,(struct sockaddr *)&server,sizeof(server));
    listen(listenfd,20);
    printf("Accetpting connections..\n");

    while(1){
      clientaddr_len = sizeof(client);
      connectfd = accept(listenfd,(struct sockaddr *)&client,&clientaddr_len);
      if(connectfd < 0){
        printf("accept error!\n");
        close(listenfd);
        exit(-1);
      }
      //POKER_DESK *pdesk = NULL;
      printf("client connectd, connectfd = %d \n",connectfd);
      pid_t pid = fork();
      if(pid < 0){
         printf("fork error!\n");
         exit(-1);
      }else if(0 == pid){
          close(listenfd);
          char recv_buf[MAX_LINE]; 
          int recv_len = 0;
          memset(recv_buf,0,MAX_LINE);
          int desk_id = -1;
          while((recv_len = recv(connectfd,recv_buf,sizeof(recv_buf),0)) > 0)
          {
            desk_id = atoi(recv_buf);
            printf("receive from client: %s ,desk_id = %d\n",recv_buf,desk_id);
            if(desk_id < 0 || desk_id > MAX_POKER_DESK)
            {
               printf("error desk id\n");
            }
            else
            {
               //pdesk = setupPokerDesk(desk_id,&proom[0]); 
               setupPokerDesk(desk_id,&proom[0]); 
            } 
            send(connectfd,recv_buf,recv_len,0);
            memset(recv_buf,0,MAX_LINE);
          }
          printf("client closed!\n");
          if((proom[0].pdesk)[desk_id] != NULL)
          {
            printf("free memory...\n");
            free((proom[0].pdesk)[desk_id]);
          }
          close(connectfd);
          exit(0);
      }else{
        close(connectfd);
      }
    }
}
