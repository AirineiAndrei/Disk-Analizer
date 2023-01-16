RECURSE_TARGET := all clean remove

$(RECURSE_TARGET):
	$(MAKE) -C ./Daemon $@
	$(MAKE) -C ./da $@

#################################
# do not change past this point #
################################

# Install the daemon and the da executable in the system
# Needs sudo 
install: all
	cp ./Daemon/bin/daemon /usr/local/bin
	chmod +x /usr/local/bin/daemon

	cp ./da/bin/da /usr/local/bin
	chmod +x /usr/local/bin/da