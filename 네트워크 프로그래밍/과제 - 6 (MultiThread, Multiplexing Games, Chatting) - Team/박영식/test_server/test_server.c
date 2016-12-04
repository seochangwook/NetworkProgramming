#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>


#define MAX_CLIENT 10
#define CHATDATA 1024

#define INVALID_SOCK -1

int list_c[MAX_CLIENT];
int list_thread[MAX_CLIENT];

char escape[] = "exit";
char greeting[] = "Welcome to chatting room\n";
char CODE200[] = "Sorry No More Connection\n";

int pushClient(int);
int popClient(int);
int pushClient_thread(int);
void *do_chat(void *);
void do_test(int);

pthread_t thread;
pthread_mutex_t mutex;

main(int argc, char *argv[])
{
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int len;
	int nfds = 0;
	int i, j, n;
	fd_set read_fds;
	char chatData[CHATDATA];
	int res;

	if(argc < 2) {
		printf( "usage: %s port_number \n", argv[0]);
		exit(-1);
	}

	if(pthread_mutex_init(&mutex, NULL) != 0) {
		printf("Can not create mutex\n");
		return -1;
	}

	s_socket = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(argv[1]));

	if(bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1) {
		printf("Can not Bind\n");
		return -1;
	}

	if(listen(s_socket, MAX_CLIENT) == -1) {
		printf("listen Fail \n");
		return -1;
	}

	for(i=0; i < MAX_CLIENT; i++) 
		list_c[i] = INVALID_SOCK;
	for(i=0; i < MAX_CLIENT; i++)
		list_thread[i] = INVALID_SOCK;

	while(1) {
		nfds = s_socket;

		FD_ZERO(&read_fds);
		FD_SET(s_socket, &read_fds);
		for(i=0; i < MAX_CLIENT; i++) {
			if(list_c[i] != INVALID_SOCK) {
				FD_SET(list_c[i], &read_fds);
				if(list_c[i] > nfds) nfds = list_c[i];
			}
		}

		nfds++;

		if(select(nfds, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) {
			printf("select error\n");
			exit(1);
		}

		if(FD_ISSET(s_socket, &read_fds)) {
				len = sizeof(c_addr);
				
				if((c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &len)) > 0) {
					printf("c_scoket:%d\n", c_socket);
					
					if(c_socket == 4) {
						pushClient_thread(c_socket);
						pthread_create(&thread, NULL, do_chat, (void *) c_socket);
					}

					else if(c_socket == 5)
					{
						pushClient_thread(c_socket);
						pthread_create(&thread, NULL, do_chat, (void *)c_socket);
					}
					
					else {
						res = pushClient(c_socket);
					
						if(res < 0) {
							write(c_socket, CODE200, strlen(CODE200));
							close(c_socket);
						} 	else {
							write(c_socket, greeting , strlen(greeting));
							}	
					}

				}
		}

		for(i=0; i < MAX_CLIENT; i++) {
			
			if((list_c[i] != INVALID_SOCK) && FD_ISSET(list_c[i], &read_fds)) {
				
				memset(chatData, 0, sizeof(chatData));

				
				if((n = read(list_c[i], chatData, sizeof(chatData))) > 0) {
					
					
					for(j=0; j < MAX_CLIENT; j++) {
						
						if(list_c[i] != INVALID_SOCK) {
							write(list_c[j], chatData, n);
						}

						if(strstr(chatData, escape) != NULL) {
							popClient(list_c[i]);
							break;
						}
						
					}
				}
			}
		}
	}
}

int pushClient(int c_socket) {
	int i;

	for(i=0; i < MAX_CLIENT; i++) {
		if(list_c[i] == INVALID_SOCK) {
			list_c[i] = c_socket;
			return i;
		}
	}

	if(i == MAX_CLIENT)
		return -1;

}

int pushClient_thread(int c_socket) {
	int i;
	for(i=0; i < MAX_CLIENT; i++) {
		pthread_mutex_lock(&mutex);
		if(list_thread[i] == INVALID_SOCK) {
			list_thread[i] = c_socket;
			pthread_mutex_unlock(&mutex);
			return i;
		}

		pthread_mutex_unlock(&mutex);
	}

	if(i == MAX_CLIENT)
		return -1;
}

int popClient(int s)
{
	int i;

	close(s);

	for(i=0; i < MAX_CLIENT; i++) {
		if(s == list_c[i]) {
			list_c[i] = INVALID_SOCK;
			break;
		}
	}

	return 0;
}

void *do_chat(void *arg)
{
	int c_socket = (int) arg;
	char chatData[CHATDATA];
	int i, n;


	while(1) {
		memset(chatData, 0, sizeof(chatData));
		if((n = read(c_socket, chatData, sizeof(chatData))) > 0) {
			for(i=0; i < MAX_CLIENT; i++) {
				if(list_thread[i] != INVALID_SOCK) {
					//printf("thread chat data\n");
					write(list_thread[i], chatData, n);
				}
			}

			if(strstr(chatData, escape) != NULL) {
				popClient(c_socket);
				break;
				}
			if(strstr(chatData, "test") != NULL) {
				for(i=0; i <  MAX_CLIENT; i++) {
					if(list_thread[i] != INVALID_SOCK && c_socket == list_thread[i]) {
						do_test(list_thread[i]);
						break;
					}
				}
			}
					
		}
	}
}

void do_test(int c_socket)
{
	 write(c_socket, "process\n", strlen("process\n"));
}

	


