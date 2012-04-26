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


int requests = 1;
int concurrency = 1;

int repeats = 1;

int total = 0;
int success = 0;

long long readbytes = 0;
long long writebytes = 0;

ev::default_loop loop;

std::string ip;
int port;

int fd;
struct sockaddr_in sock_addr;

const char * testdata = "helloworld";

class CClient
{
public:
	void Init()
	{
		io.set<CClient, &CClient::__IoCallback>(this);
		io.start(fd, ev::WRITE);

		count = 0;
	}
private:
	void __IoCallback(ev::io &watcher, int revents) 
	{
		if (EV_ERROR & revents) 
		{
			if (++count == repeats)
			{
				io.stop();
			}
			
			if(++total == requests)
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
private:
	void __ReadCallback(ev::io &watcher) 
	{
		char szBuffer[1500];
		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		ssize_t nRead = recvfrom(watcher.fd, szBuffer, sizeof(szBuffer), 0, (sockaddr*)&addr, &addrlen);
		if (nRead == (ssize_t)strlen(testdata))
		{
			success++;
			readbytes += nRead;
		}
		else if (errno == EAGAIN)
		{
//			return;
		}

		if (++count == repeats)
		{
			io.stop();
		}
	
		if (++total == requests)
		{
			loop.unloop();
		}

		watcher.set(ev::WRITE);
	}

	void __WriteCallback(ev::io &watcher) 
	{
		int nWritten = sendto(watcher.fd, testdata, strlen(testdata), 0, (sockaddr*)&sock_addr, sizeof(struct sockaddr_in));
		if (nWritten != (int)strlen(testdata))
		{
			if (errno == EAGAIN)
			{
				return;
			}
			else
			{
				if (++count == repeats)
				{
					io.stop();
				}
			
				if (++total == requests)
				{
					loop.unloop();
				}
			}
		}
		
		writebytes += nWritten;
		watcher.set(ev::READ);
	}
private:
	ev::io io;	
	int count;
};

void __SigCallback(ev::sig &signal, int revents)
{
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("usage: -n requests -c concurrency ip:port\n");
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'n':
				requests = atoi(argv[i+1]);
				break;
			case 'c':
				concurrency = atoi(argv[i+1]);
				break;
			default:
				break;
			}
			i++;
		}
		else
		{
			std::string url = argv[i];
			int pos = url.find(':');
			ip = url.substr(0, pos);
			port = atoi(url.c_str()+pos+1);
		}
	}


	repeats = requests/concurrency;

	memset((struct sockaddr*)&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());

	struct timeval tv_start, tv_end;
	gettimeofday(&tv_start, NULL);

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd == -1) 
	{
		perror("CreateSocket");
		return 0;
	}

	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	for (int i = 0; i < concurrency; ++i)
	{
		CClient * pClient = new CClient();
		pClient->Init();
	}

	ev::sig sig;
	sig.set<&__SigCallback>();
	sig.start(SIGPIPE);
	
	loop.loop();

	gettimeofday(&tv_end, NULL);

	int nUsec = (tv_end.tv_sec-tv_start.tv_sec) * 1000000 + tv_end.tv_usec-tv_start.tv_usec;
	printf("success: \033[1;33m%d\033[0m, fail: \033[1;31m%d\033[0m, time: %d.%06d s\nread: %lld bytes, write: %lld bytes, readrate: %lld KB/s, writerate: %lld KB/s\n", 
		success, requests-success, nUsec/1000000, nUsec%1000000, readbytes, writebytes, readbytes*1000000/nUsec/1024, writebytes*1000000/nUsec/1024);

	return 0;
}

