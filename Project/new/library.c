#include <sys/un.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include "clipboard.h"
#include "connection.h"

int clipboard_connect(char * clipboard_dir){
	int sfd;
	struct sockaddr_un my_addr;
	char pathSocket[108];
	sprintf(pathSocket, "./%s", CLIPBOARD_SOCKET);

	if((sfd = socket(AF_UNIX, SOCK_STREAM, 0) ) == -1){
		printf("Couldn't open socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}


	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	strncpy(my_addr.sun_path, pathSocket, sizeof(my_addr.sun_path)-1);

	if(connect(sfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_un))== -1){
		printf("Couldn't open socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	return sfd;
}

int clipboard_copy(int clipboard_id, int region, void *buf, size_t count){
	if(region < 0 || region > 9) return -1;

	int finalsize = count+sizeof(char)+sizeof(char);
	
	// C|10|olawjmidoanwdanwjabwdjawdawd
	char *msg = (char*)malloc(finalsize);

	msg[0]='C';
	msg[1]=region+'0';
	memcpy(msg+2, buf, count);
	
	printf("write:%ld\n", finalsize);

	sendMsg(clipboard_id, msg, finalsize);

	return 0;
}


int clipboard_paste(int clipboard_id, int region, void *buf, size_t count){
	if(region < 0 || region > 9) return -1;

	int finalsize = count+sizeof(char)+sizeof(char);
	char toSend[2];
	void * p = NULL;
	int r = 0;
	
	// P|10|19
	toSend[0]='P';
	toSend[1]=region+'0';
	if(sendMsg(clipboard_id, toSend, 2) == -1)
		return 0;

	// 28|olawjmidoanwdanwjabwdjawdawd|
	if((r = recvMsg(clipboard_id, (void**)&p)) == -1){
		return 0;
	}
	printf("\nreceived %d bytes\n",r);

	count = (r<count)?r:count;
	printf("Received from [1] - %s||\n", p);
	memcpy(buf, p, count);
	
	free(p);

	return count;
}
