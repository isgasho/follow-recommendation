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

objects = \
	instance-speed-history-api.o \
	instance-speed-cron.o \
	instance-first-toot-cron.o \
	archaeology-instance-speed.o \
	distsn.o
cgi_binaries = \
	distsn-instance-speed-history-api.cgi
cron_binaries = \
	distsn-instance-speed-cron \
	distsn-instance-first-toot-cron
cli_binaries = \
	distsn-archaeology-instance-speed
binaries = $(cgi_binaries) $(cron_binaries) $(cli_binaries)
cgi_scripts = \
	distsn-instance-speed-api.cgi \
	distsn-instance-speed-history-files-api.cgi \
	distsn-instance-first-toot-api.cgi \
	distsn-top1000-html.cgi
	
cgis = $(cgi_binaries) $(cgi_scripts)

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

all: $(binaries)

distsn-instance-speed-cron: distsn.o instance-speed-cron.o
	$(CXX) distsn.o instance-speed-cron.o -lcurl $(CFLAGS) -o $@

distsn-instance-speed-history-api.cgi: distsn.o instance-speed-history-api.o
	$(CXX) distsn.o instance-speed-history-api.o -lcurl $(CFLAGS) -o $@

distsn-instance-first-toot-cron: distsn.o instance-first-toot-cron.o
	$(CXX) distsn.o instance-first-toot-cron.o -lcurl $(CFLAGS) -o $@

distsn-archaeology-instance-speed: distsn.o archaeology-instance-speed.o
	$(CXX) distsn.o archaeology-instance-speed.o -lcurl $(CFLAGS) -o $@

install: $(cgis)
	$(INSTALL_PROGRAM) $(cgis) --target-directory=$(cgidir)

uninstall:
	-$(RM) $(addprefix $(cgidir)/,$(cgis))

initialize:
	$(MKDIR) -p /var/lib/distsn/instance-speed
	$(MKDIR) -p /var/lib/distsn/instance-speed-cron
	$(MKDIR) -p /var/lib/distsn/instance-first-toot
	$(CHMOD) -R a+rwx /var/lib/distsn

refresh:
	-$(RM) /var/lib/distsn/instance-speed/*
	-$(RM) /var/lib/distsn/instance-speed-cron/*

clean:
	-$(RM) $(objects) $(binaries)

activate-cgi:
	a2enmod cgi
	service apache2 restart


