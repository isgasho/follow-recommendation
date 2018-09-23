.PHONY: all clean install uninstall initialize refresh activate-cgi

srcdir = .

CFLAGS = -std=c++14 -Wall -Wpointer-arith -O2 -fno-builtin -g

VPATH = $(srcdir)

SHELL = /bin/sh
RM = /bin/rm -vf
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m a=rx
INSTALL_DATA = $(INSTALL) -m a=r
MKDIR = mkdir
RMDIR = rmdir
CHMOD = chmod

cgidir = /usr/lib/cgi-bin
bindir = /usr/local/bin

objects = \
	instance-speed-cron.o \
	instance-first-toot-cron.o \
	distsn.o \
	instances-implementation-api.o

cgi_binaries = \
	distsn-instances-implementation-api.cgi

cron_binaries = \
	distsn-instance-speed-cron \
	distsn-instance-first-toot-cron \
	distsn-pleroma-instances-cron

cron_scripts = \
	distsn-https-renew-cron

cli_binaries =

binaries = $(cgi_binaries) $(cron_binaries) $(cli_binaries)

cgi_scripts = \
	distsn-instance-speed-api.cgi \
	distsn-instance-first-toot-api.cgi \
	distsn-top1000-html.cgi \
	distsn-pleroma-instances-api.cgi
	
cgis = $(cgi_binaries) $(cgi_scripts)

crons = $(cron_binaries) $(cron_scripts)

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

all: $(binaries)

distsn-instance-speed-cron: distsn.o instance-speed-cron.o
	$(CXX) distsn.o instance-speed-cron.o -lsocialnet-1 -ltinyxml2 -lcurl $(CFLAGS) -o $@

distsn-instance-first-toot-cron: distsn.o instance-first-toot-cron.o
	$(CXX) distsn.o instance-first-toot-cron.o -lsocialnet-1 -ltinyxml2 -lcurl $(CFLAGS) -o $@

distsn-pleroma-instances-cron: distsn.o pleroma-instances-cron.o
	$(CXX) distsn.o pleroma-instances-cron.o -lsocialnet-1 -ltinyxml2 -lcurl $(CFLAGS) -o $@

distsn-instances-implementation-api.cgi: distsn.o instances-implementation-api.o
	$(CXX) distsn.o instances-implementation-api.o -lsocialnet-1 -ltinyxml2 -lcurl $(CFLAGS) -o $@

install: $(cgis)
	$(INSTALL_PROGRAM) $(cgis) --target-directory=$(cgidir)
	$(INSTALL_PROGRAM) $(crons) --target-directory=$(bindir)

uninstall:
	-$(RM) $(addprefix $(cgidir)/,$(cgis))
	-$(RM) $(addprefix $(bindir)/,$(crons))

initialize:
	$(MKDIR) -p /var/lib/distsn/instance-speed
	$(MKDIR) -p /var/lib/distsn/instance-speed-cron
	$(MKDIR) -p /var/lib/distsn/instance-first-toot
	$(CHMOD) -R a+rwx /var/lib/distsn

refresh:
	-$(RM) /var/lib/distsn/instance-speed/*
	-$(RM) /var/lib/distsn/instance-speed-cron/*
	-$(RM) /var/lib/distsn/*

clean:
	-$(RM) $(objects) $(binaries)

activate-cgi:
	a2enmod cgi
	service apache2 restart


