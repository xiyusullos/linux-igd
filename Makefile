CC=g++
INCLUDES= -I/usr/include/upnp 
LIBS= -lpthread  /usr/lib/libupnp.so


ifeq ($(DEBUG),1)
OPT = -g -O2
else
OPT = -O2
endif

CFLAGS += -Wall -g $(OPT)

APPS = upnpd

all: $(APPS)

upnpd:  gate.o gateway.o sample_util.o ipcon.o portmap.o pmlist.o
	$(CC)  $(CFLAGS) gate.o gateway.o sample_util.o ipcon.o portmap.o pmlist.o $(LIBS) -o  $@ 
	@echo "make $@ finished on `date`"

%.o:	%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o $(APPS)

install: upnpd
	@install -d /etc/linuxigd
	@install etc/* /etc/linuxigd
	@install upnpd /usr/bin

