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
#include <algorithm>
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

struct sockaddr_in sock_addr;

std::string testdata ="helloworld";

class CClient
{
public:
	void Init()
	{
		fd = socket(PF_INET, SOCK_STREAM, 0);
		if (fd == -1) 
		{
			perror("CreateSocket");

			total += repeats;
			if(total == requests)
			{
				loop.unloop();
			}
			
			return;
		}

		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
		int reuse = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
		struct linger slinger = {1, 0};   
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&slinger, sizeof(slinger)); 

		connect(fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
		
		io.set<CClient, &CClient::__IoCallback>(this);
		io.start(fd, ev::WRITE);

		count = 0;
		readlen = writelen = 0;
	}
private:	
	void __Close()
	{
		shutdown(fd, SHUT_RDWR);
		close(fd);
		io.stop();
	}

	void __IoCallback(ev::io &watcher, int revents) 
	{
		if (EV_ERROR & revents) 
		{
			__Stats();
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

		ssize_t nRead = recv(watcher.fd, szBuffer, sizeof(szBuffer)-1, 0);
		if (nRead <= 0)
		{
			if (errno != EAGAIN)
			{
				__Stats();	
			}
			return;
		}

		readbytes += nRead;

		readbuf.insert(readbuf.end(), szBuffer, szBuffer+nRead);

		if (readbuf.size() == writebuf.size())
		{
			++success;
			__Stats();
		}

	}

	void __WriteCallback(ev::io &watcher) 
	{
		if (writebuf.empty())
		{
			char szSeq[20];
			snprintf(szSeq, 20, "%d", total);
			int nPktLen = (int)testdata.size() + strlen(szSeq);
			writebuf.insert(writebuf.end(), (char*)&nPktLen, (char*)&nPktLen+sizeof(int));
			writebuf += (testdata + szSeq);
		}
		
		int nWritten = write(watcher.fd, writebuf.c_str()+writelen, writebuf.size()-writelen);

		if (nWritten <= 0)
		{
			if (errno != EAGAIN)
			{
				__Stats();
			}
			return;
		}

		writebytes += nWritten;
		
		writelen += nWritten;
		if (writelen >= (int)writebuf.size())
		{
			watcher.set(ev::READ);
		}
	}

	void __Stats()
	{
		readlen = writelen = 0;
		readbuf.clear();
		writebuf.clear();
		
		if (++total == requests)
		{
			loop.unloop();
		}

		if (++count == repeats)
		{
			__Close();
		}
		else
		{
			io.set(ev::WRITE);
		}
	}
private:
	int fd;
	ev::io io;	
	int count;
	int readlen;
	int writelen;
	std::string readbuf;
	std::string writebuf;
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

