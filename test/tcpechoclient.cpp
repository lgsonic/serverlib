#include <ev++.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <resolv.h>
#include <errno.h>
#include <queue>
#include <map>
#include <sys/time.h>
#include <arpa/inet.h>


static int clients = 0;

static int total = 0;
static int success = 0;

ev::default_loop loop;

const char * testdata = "helloworld";

class CClient
{
public:
	void __IoCallback(ev::io &watcher, int revents) 
	{
		if (EV_ERROR & revents) 
		{
			shutdown(watcher.fd, SHUT_RDWR);
			close(watcher.fd);
			watcher.stop();

			total++;
			if(total == clients)
			{
				loop.unloop();
			}
		
			return;
		}

		if (revents & EV_READ)
			__ReadCallback(watcher);

		if (revents & EV_WRITE)
			__WriteCallback(watcher);
	}

	void __ReadCallback(ev::io &watcher) 
	{
		char szBuffer[4096];

		ssize_t nRead = recv(watcher.fd, szBuffer, sizeof(szBuffer), 0);
		if(nRead == (ssize_t)strlen(testdata))
		{
			success++;
		}

		shutdown(watcher.fd, SHUT_RDWR);
		close(watcher.fd);
		watcher.stop();
			
		total++;
		if(total == clients)
		{
			loop.unloop();
		}
	}

	void __WriteCallback(ev::io &watcher) 
	{
		int nWriten = write(watcher.fd, testdata, strlen(testdata));
		
		if(nWriten != (ssize_t)strlen(testdata))
		{
			shutdown(watcher.fd, SHUT_RDWR);
			close(watcher.fd);
			watcher.stop();

			total++;
			if(total == clients)
			{
				loop.unloop();
			}
			return;
		}

		watcher.set(ev::READ);
	}
};

void __SigCallback(ev::sig &signal, int revents)
{
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		printf("usage: ip port clients\n");
		return 0;
	}
	
	ev::io *ioarray;	
	ev::sig sig;
	int *fd;
	struct sockaddr_in sock_addr;
	struct timeval tv_start, tv_end;

	memset((struct sockaddr*)&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(atoi(argv[2]));
	sock_addr.sin_addr.s_addr = inet_addr(argv[1]);

	clients = atoi(argv[3]);

	ioarray = new ev::io[clients];

	fd = new int[clients];

	CClient * client = new CClient();

	gettimeofday(&tv_start, NULL);
	
	for (int i = 0; i < clients; i++)
	{
		fd[i] = socket(PF_INET, SOCK_STREAM, 0);

		if (fd[i] == -1) 
		{
			perror("CreateSocket");
			continue;
		}

		fcntl(fd[i], F_SETFL, fcntl(fd[i], F_GETFL) | O_NONBLOCK);
		int reuse = 1;
		setsockopt(fd[i], SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
		struct linger slinger = {1, 0};   
		setsockopt(fd[i], SOL_SOCKET, SO_LINGER, (char *)&slinger, sizeof(slinger)); 

		connect(fd[i], (struct sockaddr*)&sock_addr, sizeof(sock_addr));
		
		ioarray[i].set<CClient, &CClient::__IoCallback>(client);
		ioarray[i].start(fd[i], ev::WRITE);
	}

	sig.set<&__SigCallback>();
	sig.start(SIGPIPE);
	
	loop.loop();

	gettimeofday(&tv_end, NULL);

	int nUsec = (tv_end.tv_sec-tv_start.tv_sec) * 1000000 + tv_end.tv_usec-tv_start.tv_usec;
	printf("success: \033[1;33m%d\033[0m, fail: \033[1;31m%d\033[0m, time: %d.%06d s\n", success, clients-success, nUsec/1000000, nUsec%1000000);

	return 0;
}

