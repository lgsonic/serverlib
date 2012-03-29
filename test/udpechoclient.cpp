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

struct sockaddr_in sock_addr;

class CClient
{
public:
	void __IoCallback(ev::io &watcher, int revents) 
	{
		if (EV_ERROR & revents) 
		{
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
		char szBuffer[1500];
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		ssize_t nRead = recvfrom(watcher.fd, szBuffer, sizeof(szBuffer), 0, (sockaddr*)&addr, &addrlen);
		if(nRead == (ssize_t)strlen(testdata))
		{
			success++;
		}
	
		total++;
		if(total == clients)
		{
			loop.unloop();
		}
	}

	void __WriteCallback(ev::io &watcher) 
	{
		sendto(watcher.fd, testdata, strlen(testdata), 0, (sockaddr*)&sock_addr, sizeof(struct sockaddr_in));
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
	
	ev::io io;	
	ev::sig sig;
	int fd;
	struct timeval tv_start, tv_end;

	memset((struct sockaddr*)&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(atoi(argv[2]));
	sock_addr.sin_addr.s_addr = inet_addr(argv[1]);

	clients = atoi(argv[3]);

	CClient * client = new CClient();

	gettimeofday(&tv_start, NULL);

	fd = socket(PF_INET, SOCK_DGRAM, 0);

	if (fd == -1) 
	{
		perror("CreateSocket");
		return 0;
	}

	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	io.set<CClient, &CClient::__IoCallback>(client);
	io.start(fd, ev::READ|ev::WRITE);
	
	sig.set<&__SigCallback>();
	sig.start(SIGPIPE);
	
	loop.loop();

	gettimeofday(&tv_end, NULL);

	int nUsec = (tv_end.tv_sec-tv_start.tv_sec) * 1000000 + tv_end.tv_usec-tv_start.tv_usec;
	printf("success: \033[1;33m%d\033[0m, fail: \033[1;31m%d\033[0m, time: %d.%06d s\n", success, clients-success, nUsec/1000000, nUsec%1000000);

	return 0;
}

