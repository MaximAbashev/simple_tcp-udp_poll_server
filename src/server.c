#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <poll.h>

#define SRV_PORT 5555
#define CLNT_PORT 4444

int main ()
{
	short port;
	const int on = 1;
	int tcp_sock, udp_sock, from_len, ready, contact, len, i;
	int timeout_msecs = 500;
	char buf[30];
	struct sockaddr_in s_addr, clnt_addr, new_s_addr;
	struct pollfd pds[2];
/*
 * Fabricate socket and set socket options.
 */ 
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	s_addr.sin_port = htons(SRV_PORT);
/*
 * TCP-socket part.
 */
	pds[0].fd = socket (AF_INET, SOCK_STREAM, 0);
	pds[0].events = POLLIN;
	setsockopt(pds[0].fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
	if (bind (pds[0].fd, (struct sockaddr *)&s_addr, sizeof (s_addr)) < 0)
	{
		perror ("TCP bind error!\n");
		exit (1);
	}
	listen (pds[0].fd, 1);
/*
 * UDP-socket part.
 */
	pds[1].fd = socket (AF_INET, SOCK_DGRAM, 0);
	pds[1].events = POLLIN;
	if (bind (pds[1].fd, (struct sockaddr *)&s_addr, sizeof (s_addr)) < 0)
	{
		perror ("UDP bind error!\n");
		exit (1);
	}
	while (1)
	{
        ready = poll (pds, 2, timeout_msecs);
		if ( ready < 0)
		{
			printf ("Poll error!\n");
			exit (1);
		}
		if ( ready > 0 )
		{
			if (pds[ready].revents & POLLOUT | POLLIN)
			{
				switch (ready)
				{
				    case 1:
						len = sizeof (s_addr);
						contact = accept (pds[0].fd, 
                                (struct sockaddr *)&s_addr, &len);
						if(contact == (-1))
						{
							perror ("Connect error!\n");
							exit (1);
						}
						from_len = recv (contact, buf, 21, 0);
						write (1, buf, from_len);
						send (contact, "It's for TCP client!\n", 22, 0);
						close (contact);
					case 2:
						len = sizeof (s_addr);
						while (2)
						{
							from_len = recvfrom (pds[1].fd, buf, 21, 0, 
                                    (struct sockaddr *)&s_addr, &len);
							if(from_len > 0)
							{
								write (1, buf, from_len);
								break;
							}
						}
						sendto(pds[1].fd, "It's for UDP client!\n", 22, 0,
                                (struct sockaddr *)&s_addr, sizeof(s_addr));
				}
			}	
			
		}
	}	
close(pds[0].fd);
close(pds[1].fd);
return 1;
}
