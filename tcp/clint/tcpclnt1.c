#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFLEN		1024
#define DEFSRVPORT	3000
#define MAXMSGS		3
#define IPV4LOCALADDR "127.0.0.1"


int main(int argc, char * argv[])
{
	int		ret;
	int		portnum_in = 0;
	int		sfd;
	struct		sockaddr_in server;
	int		srvaddrsz = sizeof(struct sockaddr_in);
	struct		sockaddr_in fromaddr;
	socklen_t	fromaddrsz = sizeof(struct sockaddr_in);
	in_port_t	portnum = DEFSRVPORT;
	char		inbuf[BUFLEN];
	char		outbuf[BUFLEN];
	size_t		msglen;
	size_t		msgnum = 0;
	char 		*ipaddrstr = IPV4LOCALADDR;
	in_addr_t 	ipaddrbin;

	fprintf(stdout, "Connection-oriented client program ...\n");

	if (argc > 1)
	{
		portnum_in  = atoi(argv[1]);
		if (portnum <= 0)
		{
			fprintf(stderr, "Port number %d invalid, set to default value %u\n", portnum_in, DEFSRVPORT);
			portnum = DEFSRVPORT;
		}
		else
		{
			portnum = (in_port_t)portnum_in;
		}
	}


	if (argc > 2){ipaddrstr = argv[2];}

	ret = inet_pton(AF_INET, ipaddrstr, &ipaddrbin);
	if (ret == 0)
	{
		fprintf(stderr, "%s is not avalid IP address.\n", ipaddrstr);
		return -1;
	}

	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error: socket() failed, errno = %d, %s\n", errno, strerror(errno));
		return -2;
	}


	memset((void *)&server, 0, (size_t)srvaddrsz);
	server.sin_family = AF_INET;
	if (strcmp(ipaddrstr, IPV4LOCALADDR))
	{
		server.sin_addr.s_addr = ipaddrbin;
	}
	else
	{
		server.sin_addr.s_addr = htonl(INADDR_ANY);

	}
	server.sin_port = htons(portnum);




	ret = connect(sfd, (struct sockaddr *)&server, srvaddrsz);
	if (ret == -1)
	{
		fprintf(stderr, "Error: connect() failed, errno = %d, %s\n", errno, strerror(errno));
		close(sfd);
		return -3;
	}



	fprintf(stdout, "Send request messages to server(%s) at port %d\n", ipaddrstr, portnum);

	while (msgnum < MAXMSGS)
	{
		sprintf(outbuf, "%s%4lu%s", "This is request message ", ++msgnum, " from the client program.");
		msglen = strlen(outbuf);
		errno = 0;

		ret = send(sfd, outbuf, msglen, 0);
		if (ret >= 0)
		{
			if (ret == msglen)
			{
				fprintf(stdout, "\n%lu bytes of message were successfully sent.\n", msglen);
			}
			else if (ret < msglen)
			{
				fprintf(stderr, "Warning: only %u of %lu bytes were sent.\n", ret, msglen);
			}

			if (ret > 0)
			{
				errno = 0;
				inbuf[0] = '\0';
				ret = recv(sfd, inbuf, BUFLEN, 0);

				if (ret > 0)
				{
					inbuf[ret] = '\0';
					fprintf(stdout, "Received the following reply from server:\n%s\n", inbuf);
				}
				else if (ret == 0)
				{
					fprintf(stdout, "Warning: Zero bytes were received.\n");
				}
				else
				{
					fprintf(stderr, "Error: recv() failed, errno = %d, %s\n", errno, strerror(errno));
				}
			}
		}
		else
		{
			fprintf(stderr, "Error: send() failed, errno = %d, %s\n", errno, strerror(errno));
		}

		sleep(1);
	}

	close(sfd);
	return 0;
}




