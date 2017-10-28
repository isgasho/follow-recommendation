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
	user-speed-api.o \
	user-recommendation-api.o \
	instance-speed-history-api.o \
	user-speed.o \
	instance-speed-cron.o \
	instance-first-toot-cron.o \
	user-application-cron.o \
	instance-lists-api.o \
	archaeology-instance-speed.o \
	distsn.o
cgi_binaries = \
	distsn-user-speed-api.cgi \
	distsn-user-recommendation-api.cgi \
	distsn-instance-speed-history-api.cgi \
	distsn-instance-lists-api.cgi
cron_binaries = \
	distsn-user-speed \
	distsn-instance-speed-cron \
	distsn-instance-first-toot-cron \
	distsn-user-application-cron
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

distsn-user-speed: distsn.o user-speed.o
	$(CXX) distsn.o user-speed.o -lcurl $(CFLAGS) -o $@

distsn-user-speed-api.cgi: distsn.o user-speed-api.o
	$(CXX) distsn.o user-speed-api.o -lcurl $(CFLAGS) -o $@

distsn-user-recommendation-api.cgi: distsn.o user-recommendation-api.o
	$(CXX) distsn.o user-recommendation-api.o -lcurl $(CFLAGS) -o $@

distsn-instance-speed-cron: distsn.o instance-speed-cron.o
	$(CXX) distsn.o instance-speed-cron.o -lcurl $(CFLAGS) -o $@

distsn-instance-speed-history-api.cgi: distsn.o instance-speed-history-api.o
	$(CXX) distsn.o instance-speed-history-api.o $(CFLAGS) -o $@

distsn-instance-first-toot-cron: distsn.o instance-first-toot-cron.o
	$(CXX) distsn.o instance-first-toot-cron.o -lcurl $(CFLAGS) -o $@

distsn-user-application-cron: distsn.o user-application-cron.o
	$(CXX) distsn.o user-application-cron.o -lcurl $(CFLAGS) -o $@

distsn-instance-lists-api.cgi: distsn.o instance-lists-api.o
	$(CXX) distsn.o instance-lists-api.o -lcurl $(CFLAGS) -o $@

distsn-archaeology-instance-speed: distsn.o archaeology-instance-speed.o
	$(CXX) distsn.o archaeology-instance-speed.o -lcurl $(CFLAGS) -o $@

install: $(cgis)
	$(INSTALL_PROGRAM) $(cgis) --target-directory=$(cgidir)

uninstall:
	-$(RM) $(addprefix $(cgidir)/,$(cgis))

initialize:
	$(MKDIR) -p /var/lib/distsn/user-speed
	$(MKDIR) -p /var/lib/distsn/user-application
	$(MKDIR) -p /var/lib/distsn/instance-speed
	$(MKDIR) -p /var/lib/distsn/instance-speed-cron
	$(MKDIR) -p /var/lib/distsn/instance-first-toot
	$(CHMOD) -R a+rwx /var/lib/distsn

refresh:
	-$(RM) /var/lib/distsn/user-speed/*
	-$(RM) /var/lib/distsn/instance-speed/*
	-$(RM) /var/lib/distsn/instance-speed-cron/*

clean:
	-$(RM) $(objects) $(binaries)

activate-cgi:
	a2enmod cgi
	service apache2 restart


