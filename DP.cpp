#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<iostream>
#include <sys/socket.h> //for socket(), connect(), ...
#include <unistd.h>     //for close()
#include <netinet/in.h> //for internet address family
#include <arpa/inet.h>  //for sockaddr_in, inet_addr()
#include <errno.h>      //for system error numbers

#define SERVER_PORT_ID  19123

#define MESSAGESIZE        80

pthread_mutex_t myFork;

char const* ipAddress[] = {
        "147.97.156.241", "147.97.156.242",
        "147.97.156.243", "147.97.156.244",
        "147.97.156.245"
    };
    
void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs; // Get finishing time.
    //printf("the waiting time is %d \n",retTime);
    while (time(0) < retTime);    // Loop until it arrives.
}


    
void* client(void* number)
{
    waitFor(20);
    int sockfd, retcode, nread, addrlen;
    int server_port_id = SERVER_PORT_ID;
    char const* serv_host_addr="142.97.156.241";
    struct sockaddr_in my_addr, server_addr;
    char msg[MESSAGESIZE];
    unsigned int waitingTime=24;
    int philosopher=atoi((char*)number);
    srand (time(NULL));
    
    // ---------------------------------------------------------
   // Finding the adjacent philosopher
   // ---------------------------------------------------------
    if(philosopher==5)
    {
        serv_host_addr=ipAddress[0];
    }
    else
    {
        serv_host_addr=ipAddress[philosopher];
    }
    
    // ---------------------------------------------------------
   // Initialization:
   // ---------------------------------------------------------
   if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
   { 
       printf("Client: socket failed:..\n");
       exit(1); 
   }

   memset( &my_addr, 0, sizeof(my_addr));       // Zero out structure
   my_addr.sin_family = AF_INET;                // Internet address family
   my_addr.sin_port = htons(0);                 // My port, 0 means any random available one
   my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface

   // ---------------------------------------------------------
   // binding:
   // ---------------------------------------------------------
   if ( (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) )
   {
       printf("client: bind fail:..\n");
       exit(2);
   }


   // ---------------------------------------------------------
   // prepare server address
   // ---------------------------------------------------------
   memset( &server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(server_port_id);
   if (inet_aton(serv_host_addr, &(server_addr.sin_addr))==0) // get server addr   
   {  // invalid server address
       printf("Client: Invalid server address...\n");
       exit(2);
   }
   
   // ---------------------------------------------------------
   // Message Preparation  
   // ---------------------------------------------------------
   
   bool flag=true;
   while(1)
   {
    if(pthread_mutex_lock(&myFork)!=0) //unsucessful lock
    {
      // printf("unable to get the lock client \n");
       if(waitingTime>6)
       {
       waitingTime-=3;
       }
       waitFor(waitingTime);
    }
    // ---------------------------------------------------------
   // Transmission  
   // ---------------------------------------------------------
    else
    {
      strcpy(msg,  "lock");  
      retcode = sendto(sockfd, msg, strlen(msg)+1, 0,
                  (struct sockaddr *) &server_addr, sizeof(server_addr));
      if (retcode <= -1)
      {
          printf("client: sendto failed:...\n");
          exit(3); 
      }   

    // ---------------------------------------------------------
   // Get message from server 
   // ---------------------------------------------------------
      addrlen = sizeof(server_addr);   // specify the size of server_addr, this is important !!!!!
      nread = recvfrom(sockfd,msg, MESSAGESIZE, 0,
                 (struct sockaddr *) &server_addr, (socklen_t *) &addrlen);
      //printf("the value from server is %s \n",msg);
      if (nread >0)
       {
          if(strncmp(msg,"locked",6)==0)
          {
            waitingTime=rand() % (24) + 6;
            printf("\n philosopher %d is eating \n",philosopher);
            waitFor(rand() % 10 + 1);
            pthread_mutex_unlock(&myFork);
            strcpy(msg,  "unlock");  
            retcode = sendto(sockfd, msg, strlen(msg)+1, 0,
                  (struct sockaddr *) &server_addr, sizeof(server_addr));
            if (retcode <= -1)
          {
          printf("client: sendto failed:...\n");
          exit(3); 
           }
          }
          else if(strncmp(msg,"unsec",5)==0)
          {
            waitingTime-=3;
            pthread_mutex_unlock(&myFork);   
          }
          waitFor(waitingTime);
       }
    }
    
   }  
    
}



void* server(void* number)
{
  
  int sockfd;  //socket declaration
   struct sockaddr_in my_addr, client_addr;  //addresses
   int nread,        //number of bytes read
       retcode,      //returned code from a function
       addrlen;      //address length
   
   char msg[MESSAGESIZE];
   int waitingTime=rand() % (24) + 6;
   int philosopher=atoi((char*)number);

   int my_port_id = SERVER_PORT_ID;  //default port number
   // ---------------------------------------------------------
   // Initialization:
   // ---------------------------------------------------------
   //cout << "Server: creating socket\n";
   if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
   { 
        printf("Server: socket failed:..\n"); 
       exit(1); 
   }

   // cout << "Server: binding my local socket\n";
   memset( &my_addr, 0, sizeof(my_addr));    // Zero out structure
   my_addr.sin_family = AF_INET;        // Internet address family
   my_addr.sin_port = htons(my_port_id);    //My port
   my_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // Any incoming interface

   // ---------------------------------------------------------
   // binding:
   // ---------------------------------------------------------
   if ( (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) )
   { 
        printf("Client: bind failed:..\n"); 
       exit(2); 
   }   
  
   while (1)
   {
       // ---------------------------------------------------------
       // Wait for client's connection
       // ---------------------------------------------------------
       printf("\n philosopher %d is thinking....\n",philosopher);
       addrlen = sizeof(client_addr);   // need to give it the buffer size for sender's address
       nread = recvfrom(sockfd,msg, MESSAGESIZE, 0,
                  (struct sockaddr *) &client_addr, (socklen_t *) &addrlen);
       //printf("the value from client is %s \n",msg);

       if (nread >0) 
       {
           if(strncmp(msg,"lock",4)==0)
           {
                if(pthread_mutex_lock(&myFork)!=0) //unsucessful lock
                  {
                     strcpy(msg,  "unsuc");
                     retcode = sendto(sockfd,msg,strlen(msg)+1,0,
                     (struct sockaddr *) &client_addr, sizeof(client_addr));
                     if (retcode <= -1)
                     {
                     printf("\n\nError in sending server\n\n");
                     //cerr << "client: sendto failed: " << errno << "\n";
                     exit(3);
                     }
                     if(waitingTime>6)
                     {
                       waitingTime-=3;
                     }
                   waitFor(waitingTime);
                 }
                 else
                 {
                     strcpy(msg,  "locked");
                     retcode = sendto(sockfd,msg,strlen(msg)+1,0,
                     (struct sockaddr *) &client_addr, sizeof(client_addr));
                     if (retcode <= -1)
                    {
                     printf("\n\nError in sending server\n\n");
                     exit(3);
                    }
                     
                 }
                
           }
           else if(strncmp(msg,"unlock",6)==0)
           {
             pthread_mutex_unlock(&myFork);
           }
       }
           
       }
   }  

int main(int argc, char *argv[])
{
    pthread_t client_thread, server_thread;
    
    if (pthread_mutex_init(&myFork, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    // make threads
    pthread_create(&client_thread, NULL, client,(void *)argv[1]);
    pthread_create(&server_thread, NULL, server,(void *)argv[1]);

    // wait for them to finish
    pthread_join(client_thread, NULL);
    pthread_join(server_thread, NULL);
    pthread_mutex_destroy(&myFork);

    return 0;
}