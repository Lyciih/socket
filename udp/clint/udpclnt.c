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
	struct		sockaddr_in srvaddr;
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

	fprintf(stdout, "Connectionless client program ...\n");

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

	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "Error: socket() failed, errno = %d, %s\n", errno, strerror(errno));
		return -2;
	}


	memset((void *)&srvaddr, 0, (size_t)srvaddrsz);
	srvaddr.sin_family = AF_INET;
	if (ipaddrstr)
	{
		srvaddr.sin_addr.s_addr = ipaddrbin;
	}
	else
	{
		srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	}
	srvaddr.sin_port = htons(portnum);


	fprintf(stdout, "Send request messages to server(%s) at port %d\n", ipaddrstr, portnum);

	while (msgnum < MAXMSGS)
	{
		sprintf(outbuf, "%s%4lu%s", "This is request message ", ++msgnum, " from the client program.");
		msglen = strlen(outbuf);
		errno = 0;

		ret = sendto(sfd, outbuf, msglen, 0, (struct sockaddr *)&srvaddr, srvaddrsz);
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
				ret = recvfrom(sfd, inbuf, BUFLEN, 0, (struct sockaddr *)&fromaddr, &fromaddrsz);

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
					fprintf(stderr, "Error: recvfrom() failed, errno = %d, %s\n", errno, strerror(errno));
				}
			}
		}
		else
		{
			fprintf(stderr, "Error: sendto() failed, errno 123 = %d, %s\n", errno, strerror(errno));
		}

		sleep(1);
	}

	close(sfd);
	return 0;
}




