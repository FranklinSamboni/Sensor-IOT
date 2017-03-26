#ifndef _SOCKETLIB_H_
#define _SOCKETLIB_H_

	#include <netinet/in.h>

	#define SERVER_IP "52.34.55.59"
	#define SOCKET_PORT 46500

	typedef struct socketStr socketStr;
	typedef struct socketData socketData;

	struct socketStr {
		int sockfd;
		struct sockaddr_in serv_addr;
	};

	struct socketData {

	};

	int openSOCKET(char * ip_server, int port);
	int readSOCKET(char * buffer);
	int writeSOCKET(const char * buffer);
	int closeSOCKET();

	void errorSocket(char *msgError);

#endif
