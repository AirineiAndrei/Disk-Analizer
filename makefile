all: daemon worker da

daemon:
	mkdir -p bin
	gcc ./Daemon/daemon.c -o ./bin/daemon

worker:
	mkdir -p bin
	gcc ./Worker/analyzer.c -o ./bin/analyzer

da:
	mkdir -p bin
	gcc ./da.c -o ./bin/da

#################################
# do not change past this point #
################################

clean:
	rm -r -f ./bin

# Install the daemon and the da executable in the system
# Needs sudo 
install: all
	cp ./bin/daemon /usr/local/bin
	chmod +x /usr/local/bin/daemon

	cp ./bin/da /usr/local/bin
	chmod +x /usr/local/bin/da
