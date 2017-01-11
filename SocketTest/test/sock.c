#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main()
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct in_addr ipv4addr;

    char buffer[256];
    /*if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }*/
    portno = 46500;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(0);
    }

    inet_pton(AF_INET, "35.163.18.161", &ipv4addr);
    server = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    printf("Host name: %s\n", server->h_name);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
        exit(0);
    }
	while(1){
  /*  printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = send(sockfd , buffer , strlen(buffer) , 0);
    
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);*/
    n = recv(sockfd , buffer , 2000 , 0);
    //n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("Recibido: %s\n",buffer);}
    close(sockfd);
    return 0;
}
