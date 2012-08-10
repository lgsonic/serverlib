CC=gcc
CXX=g++
AR=ar
LN=ln -s
PRJPATH=$(shell pwd)
CPPFLAGS = -g -O2 -Wall -fPIC -fno-strict-aliasing -I$(PRJPATH) -I$(PRJPATH)/ev
LDLIBS = -lpthread -L$(PRJPATH)/ -lserver -lclient -L$(PRJPATH)/ev -lev
DY_LDFLAGS = -shared -fPIC
ST_LDFLAGS = crs
VERSION_INFO = .0.1.8


COMPILE.CXX = $(CXX) $(CPPFLAGS) -c
COMPILE.C = $(CC) $(CPPFLAGS) -c
LINK.SHARE = $(CXX) $(DY_LDFLAGS)
LINK.STATIC = $(AR) $(ST_LDFLAGS)


SERVERCOMMONOBJS=iserver.o tcpserver.o udpserver.o commonserver.o
SERVERLIB=libserver.a
SERVERLIBSHARE=libserver.so
SERVERLIBSHAREV=libserver.so$(VERSION_INFO)

CLIENTCOMMONOBJS=iclient.o tcpclient.o udpclient.o
CLIENTLIB=libclient.a
CLIENTLIBSHARE=libclient.so
CLIENTLIBSHAREV=libclient.so$(VERSION_INFO)

TESTOBJS=test/tcpechoserver.o test/tcpechoclient.o test/udpechoserver.o test/udpechoclient.o test/tcpproxy.o test/tcpproxytest.o test/tcpcommonechoserver.o test/udpproxy.o test/udpproxytest.o
TCPECHOSERVER=test/tcpechoserver
TCPECHOCLIENT=test/tcpechoclient
UDPECHOSERVER=test/udpechoserver
UDPECHOCLIENT=test/udpechoclient
TCPPROXY=test/tcpproxy
TCPPROXYTEST=test/tcpproxytest
TCPCOMMONECHOSERVER=test/tcpcommonechoserver
UDPPROXY=test/udpproxy
UDPPROXYTEST=test/udpproxytest
TESTAPPS=$(TCPECHOSERVER) $(TCPECHOCLIENT) $(UDPECHOSERVER) $(UDPECHOCLIENT) $(TCPPROXY) $(TCPPROXYTEST) $(TCPCOMMONECHOSERVER) $(UDPPROXY) $(UDPPROXYTEST)

all: $(SERVERLIB) $(SERVERLIBSHARE) $(CLIENTLIB) $(CLIENTLIBSHARE) 

test: $(TESTAPPS)

%.o:%.cpp %.h
	$(COMPILE.CXX) $< -o $@

$(SERVERLIB): $(SERVERCOMMONOBJS)
	$(LINK.STATIC) $@ $(SERVERCOMMONOBJS)

$(SERVERLIBSHARE): $(SERVERCOMMONOBJS)
	$(LINK.SHARE) -o $(SERVERLIBSHAREV) $(SERVERCOMMONOBJS)
	$(LN) $(SERVERLIBSHAREV) $(SERVERLIBSHARE)

$(CLIENTLIB): $(CLIENTCOMMONOBJS)
	$(LINK.STATIC) $@ $(CLIENTCOMMONOBJS)

$(CLIENTLIBSHARE): $(CLIENTCOMMONOBJS)
	$(LINK.SHARE) -o $(CLIENTLIBSHAREV) $(CLIENTCOMMONOBJS)
	$(LN) $(CLIENTLIBSHAREV) $(CLIENTLIBSHARE)

$(TCPECHOSERVER): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/tcpechoserver.o $(LDLIBS)

$(TCPECHOCLIENT): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/tcpechoclient.o $(LDLIBS)

$(UDPECHOSERVER): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/udpechoserver.o $(LDLIBS)

$(UDPECHOCLIENT): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/udpechoclient.o $(LDLIBS)

$(TCPPROXY): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/tcpproxy.o $(LDLIBS) 

$(TCPPROXYTEST): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/tcpproxytest.o $(LDLIBS) 

$(TCPCOMMONECHOSERVER): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/tcpcommonechoserver.o $(LDLIBS)

$(UDPPROXY): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/udpproxy.o $(LDLIBS) 

$(UDPPROXYTEST): $(TESTOBJS) $(SERVERLIB) $(CLIENTLIB)
	$(CXX) -o $@ test/udpproxytest.o $(LDLIBS) 


clean:
	rm -f $(SERVERCOMMONOBJS) $(SERVERLIB) $(SERVERLIBSHARE)  $(SERVERLIBSHAREV) $(CLIENTCOMMONOBJS) $(CLIENTLIB) $(CLIENTLIBSHARE) $(CLIENTLIBSHAREV) $(TESTOBJS) $(TESTAPPS)

