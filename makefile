CC=gcc
CXX=g++
AR=ar
PRJPATH=$(shell pwd)
CPPFLAGS = -g -O2 -Wall -fPIC -I$(PRJPATH) -I$(PRJPATH)/ev
LDLIBS = -lpthread -L$(PRJPATH)/ -lserver -L$(PRJPATH)/ev -lev
DY_LDFLAGS = -shared -fPIC
ST_LDFLAGS = crs


COMPILE.CXX = $(CXX) $(CPPFLAGS) -c
COMPILE.C = $(CC) $(CPPFLAGS) -c
LINK.SHARE = $(CXX) $(DY_LDFLAGS)
LINK.STATIC = $(AR) $(ST_LDFLAGS)


COMMONOBJS=iserver.o tcpserver.o udpserver.o
SERVERLIB=libserver.a
SERVERLIBSHARE=libserver.so

TESTOBJS=test/tcpechoserver.o test/tcpechoclient.o test/udpechoserver.o test/udpechoclient.o
TCPECHOSERVER=test/tcpechoserver
TCPECHOCLIENT=test/tcpechoclient
UDPECHOSERVER=test/udpechoserver
UDPECHOCLIENT=test/udpechoclient

all: $(SERVERLIB) $(SERVERLIBSHARE) 

test: $(TCPECHOSERVER) $(TCPECHOCLIENT) $(UDPECHOSERVER) $(UDPECHOCLIENT)

%.o:%.cpp %.h
	$(COMPILE.CXX) $< -o $@

$(SERVERLIB): $(COMMONOBJS)
	$(LINK.STATIC) $@ $(COMMONOBJS)

$(SERVERLIBSHARE): $(COMMONOBJS)
	$(LINK.SHARE) -o $@ $(COMMONOBJS)

$(TCPECHOSERVER): $(TESTOBJS) $(SERVERLIB)
	$(CXX) -o $@ test/tcpechoserver.o $(LDLIBS)

$(TCPECHOCLIENT): $(TESTOBJS) $(SERVERLIB)
	$(CXX) -o $@ test/tcpechoclient.o  $(LDLIBS)

$(UDPECHOSERVER): $(TESTOBJS) $(SERVERLIB)
	$(CXX) -o $@ test/udpechoserver.o $(LDLIBS)

$(UDPECHOCLIENT): $(TESTOBJS) $(SERVERLIB)
	$(CXX) -o $@ test/udpechoclient.o  $(LDLIBS)


clean:
	rm -f $(COMMONOBJS) $(SERVERLIB) $(SERVERLIBSHARE)  $(TESTOBJS) $(TCPECHOSERVER) $(TCPECHOCLIENT) $(UDPECHOSERVER) $(UDPECHOCLIENT)

