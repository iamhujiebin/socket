#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MAXEVENTS 100

int initserver(int port) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	bind(sock, (sockaddr *) &servaddr, sizeof(servaddr));
	listen(sock, 5);
	return sock;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Using ./tcpepoll port\n");
		return -1;
	}
	int listensock = initserver(atoi(argv[1]));
	printf("listensock=%d\n", listensock);

	if (listensock < 0) {
		printf("initserver() failed.\n");
		return -1;
	}
	int epollfd;
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	// create fd
	epollfd = epoll_create(1);
	// add listen fd event
	struct epoll_event ev;
	ev.data.fd = listensock;
	ev.events = EPOLLIN;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,listensock,&ev);
	while(true){
		// arr for coming events
		struct epoll_event events[MAXEVENTS];
		// wait for events
		int nfds = epoll_wait(epollfd,events,MAXEVENTS,-1);	
		if (nfds < 0){
			break;
		}
		if (nfds == 0){
			printf("timeout \n");
			continue;
		}
		// traverse
		for (int i = 0;i<nfds;i++){
			if ((events[i].data.fd == listensock) & (events[i].events & EPOLLIN)){
				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
				if (clientsock < 0)
				{
					printf("accept() failed.\n"); continue;
				}
				// add clientfd to epoll
				memset(&ev,0,sizeof(struct epoll_event));
				ev.data.fd = clientsock;
				ev.events = EPOLLIN;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);
				printf ("client(socket=%d) connected ok.\n",clientsock);
				continue;
			} else if (events[i].events & EPOLLIN){
				char buffer[1024];
				memset(buffer,0,sizeof(buffer));
				ssize_t isize=read(events[i].data.fd,buffer,sizeof(buffer));
				if (isize <=0)
				{
					printf("client(eventfd=%d) disconnected.\n",events[i].data.fd);

					memset(&ev,0,sizeof(struct epoll_event));
					ev.events = EPOLLIN;
					ev.data.fd = events[i].data.fd;
					epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
					close(events[i].data.fd);
					continue;
				}

				printf("recv(eventfd=%d,size=%d):%s\n",events[i].data.fd,isize,buffer);

				write(events[i].data.fd,buffer,strlen(buffer));
			}
		}
	}
	close(epollfd);
	return 0;
}
