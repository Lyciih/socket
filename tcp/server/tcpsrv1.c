#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>



#define DEFSRVPORT	3001
#define BACKLOG		50
#define BUFLEN		1024


int main(int argc, char *argv[])
{
	int		ret;
	int		portnum_in = 0;
	int		sfd;
	int		newsock;
	struct		sockaddr_in srvaddr;
	int		srvaddrsz = sizeof(struct sockaddr_in);
	in_port_t	portnum = DEFSRVPORT;

	char		inbuf[BUFLEN];
	char		outbuf[BUFLEN];
	size_t		msglen;
	
	struct		sockaddr_in clntaddr;
	socklen_t	clntaddrsz = sizeof(struct sockaddr_in);

	unsigned int 	msgcnt;



	fprintf(stdout, "Connection-oriented server program ...\n");

	if (argc > 1)
	{
		portnum_in = atoi(argv[1]);
		if (portnum_in <= 0)
		{
			fprintf(stderr, "Port number %d invalid, set to default value %u\n",portnum_in, DEFSRVPORT);
			portnum = DEFSRVPORT;
		}
		else
		{
			portnum  = (in_port_t)portnum_in;
		}
	}



	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error: socket() failed, errno=%d, %s\n", errno, strerror(errno));
		return -1;
	}



	memset((void *)&srvaddr, 0, (size_t)srvaddrsz);
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvaddr.sin_port = htons(portnum);


	if ((ret = bind(sfd, (struct sockaddr *)&srvaddr, srvaddrsz)) != 0)
	{
		fprintf(stderr, "Error: bind() failed, errno = %d, %s\n", errno, strerror(errno));
		close(sfd);
		return -2;
	}

	if (listen(sfd, BACKLOG) == -1)
	{
		fprintf(stderr, "Error: listen() failed, errno = %d, %s\n", errno, strerror(errno));
		close(sfd);
		return -3;
	}



	while (1)
	{
		fprintf(stdout, "\nListening at port number %u ...\n", portnum);
		newsock = accept(sfd, (struct  sockaddr *)&clntaddr, &clntaddrsz);
		if (newsock < 0)
		{
			fprintf(stderr, "Error: accept() failed, errno = %d, %s\n", errno, strerror(errno));
			close(sfd);
			return -4;
		}

		fprintf(stdout, "Client Connected.\n");

		msgcnt  = 1;

	

		while (1)
		{
			errno = 0;
			inbuf[0] = '\0';
			ret = recv(newsock, inbuf, BUFLEN, 0);


			if (ret > 0)
			{
				inbuf[ret] = '\0';
				fprintf(stdout, "\nReceived the following request from client:\n%s\n", inbuf);

				sprintf(outbuf, "This is reply #%3u from the server program.", msgcnt++);
				msglen = strlen(outbuf);

				errno = 0;
				ret = send(newsock, outbuf, msglen, 0);
				
				if (ret == -1)
				{
					fprintf(stderr, "Error: send() failed, errno = %d, %s\n", errno, strerror(errno));
				}
				else
				{
					fprintf(stdout, "%u of %lu bytes of the reply was sent.\n", ret, msglen);
				}
			}
			else if (ret < 0)
			{
				fprintf(stderr, "Error: recv() failed, errno = %d, %s\n", errno, strerror(errno));
				break;
			}
			else
			{
				fprintf(stdout, "The clinet may have dissconnected.\n");
				break;
			}

		}
		close(newsock);
	}
	return 0;
}




