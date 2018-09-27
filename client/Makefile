.PHONY: install uninstall

srcdir = .

htmldir = /var/www/html

VPATH = $(srcdir)

SHELL = /bin/sh
RM = /bin/rm -vf
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m a=rx
INSTALL_DATA = $(INSTALL) -m a=r
MKDIR = mkdir -p
RMDIR = rmdir

www_files = \
	index.html \
	follow-recommendation.html \
	user-speed.html \
	instance-speed.html instance-speed.js \
	instance-new.html instance-new.js \
	fake-mastodon.css \
	kasaikuminkan.jpg google214b9ca3f27d115d.html \
	missing.svg \
	missing.png \
	user-speed-raw.html \
	pleroma-instances.html \
	pleroma-instances.js \
	fake-pleroma-light.css \
	gnusocial-instances.html \
	misskey-instances.html \
	instances-implementation.js \
	gnusocial-instances.js \
	misskey-instances.js \
	gnusocial.png

install: $(www_files)
	$(INSTALL_DATA) $(www_files) --target-directory=$(htmldir)

uninstall:
	-$(RM) $(addprefix $(htmldir)/,$(www_files))


