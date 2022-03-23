/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

/* request and response */
void dostuff(int sock){
    /* read request */
    char buffer[1024];
    bzero(buffer,1024);
    int n = read(sock,buffer,1023);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the request message: %s\n",buffer);

    /* extract file_name, http_ver from request msg */
    char file_name[20];// ex) text.html image.jpg
    bzero(file_name,20);
    char http_ver[10];//ex) HTTP/1.1
    bzero(http_ver,10);
    strtok(buffer,"/");
    char *sample;
    sample = strtok(NULL," ");
    strcat(file_name,sample);
    sample = strtok(NULL,"\r");
    strcat(http_ver,sample);
    /* file open */
    FILE *fp;
    fp = fopen(file_name, "rb");
    char *file_status = malloc(sizeof(char)*40); // ex) 200 OK, 404..
    bzero(file_status,40);
    /* check file status */
    if(fp == NULL){
	snprintf(file_status,20,"%s 404 not found\r\n",http_ver);
	n = write(sock, file_status, strlen(file_status)); // send 404 not found
   	if (n < 0) error("ERROR data sending to socket3");
	return;
    }
    else file_status = "200 OK";
    fseek(fp, 0, SEEK_END);
    int send_fsize = ftell(fp); // record file size
    fseek(fp, 0, SEEK_SET);

    /* select content-type */
    strtok(file_name,".");
    sample = strtok(NULL,"\0");
    char *c_type = malloc(sizeof(char)*(50));
    bzero(c_type,50);
    if(strcmp(sample,"html")==0) snprintf(c_type,50,"text/html");
    else if(strcmp(sample,"gif")==0) snprintf(c_type,50,"image/gif");
    else if(strcmp(sample,"jpeg")==0 || strcmp(sample,"jpg")==0) snprintf(c_type,50,"image/jpeg");
    else if(strcmp(sample,"mp3")==0) snprintf(c_type,50,"audio/mp3");
    else if(strcmp(sample,"pdf")==0) snprintf(c_type,50,"application/pdf");
    else if(strcmp(sample,"png")==0) snprintf(c_type,50,"image/png");
    else if(strcmp(sample,"ico")==0) snprintf(c_type,50,"image/x-icon");
    /* http response msg */
    char *hpbuf = malloc(sizeof(char)*(256));
    bzero(hpbuf,256);
    snprintf(hpbuf,256,"%s %s\r\n"
    "Content-Type: %s\r\n"
    "Keep-Alive: timeout=15, max=100\r\n"
    "Connection: Keep-Alive\r\n"
    "\r\n",
    http_ver, file_status, c_type);
    /* send http response header */
    printf("response msg : \n%s",hpbuf);
    n = write(sock, hpbuf, strlen(hpbuf));
    if (n < 0) error("ERROR data sending to socket1");
    /* send http response content */
    char *file_content = malloc(sizeof(char)*(1024));
    bzero(file_content,1024);
    int nsize, fpsize;
    nsize = send_fsize;
    /* split transmission */
    while(nsize > 0){
	    fpsize = fread(file_content, sizeof(char), 1023, fp);
	    nsize -= fpsize;
	    n = write(sock, file_content, fpsize); // send content
	    if (n < 0) error("ERROR data sending to socket2");
	}
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, pid; //descriptors return from socket and accept system calls
    int portno; // port number
    socklen_t clilen;
     
    /*sockaddr_in: Structure Containing an Internet Address(AF_INET)*/
    struct sockaddr_in serv_addr, cli_addr;

    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    /*Create a new socket
      AF_INET: Address Domain is Internet (IPv4)
      SOCK_STREAM: Socket Type is STREAM Socket (TCP) */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
       error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));// init zero
    portno = atoi(argv[1]); //atoi converts from String to Integer
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
    serv_addr.sin_port = htons(portno); //convert from host to network byte order

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
             error("ERROR on binding");

    listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5


    clilen = sizeof(cli_addr);
    /* run indefinitely */
    while(1){
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    	if (newsockfd < 0) error("ERROR on accept");
	    pid = fork();
	    if(pid<0) error("ERROR on fork");
	    else if(pid==0){
	    	close(sockfd);
	        dostuff(newsockfd);
	        sleep(5);
	    	exit(0);
	    }
	    else close(newsockfd);
    }
    close(sockfd);
    return 0;
}
