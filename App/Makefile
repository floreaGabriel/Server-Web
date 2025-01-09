###############################################################################
# MARK: ALL
###############################################################################

# Create top level static library and all sub-libraries
all: cJSON Main Server-files main_exec


###############################################################################
# MARK: EXECUTABLE
###############################################################################

# Build the main executable
main_exec: Main
	gcc -o main main.c -L. -lmini_server -L./cJSON -lcjson -lz


###############################################################################
# MARK: MAIN
###############################################################################

# Creates just the top level static library
Main: Server-filesSub cJSON
	ar rcs libmini_server.a  Server.o ThreadPool.o ServerConfig.o



###############################################################################
# MARK: NETWORKING
###############################################################################

# Creates the networking library
Server-files: Server-filesSub
	ar rcs Server-files/Server-files.a Server.o ThreadPool.o ServerConfig.o

# Sub components of the networking library
Server-filesSub: Server ThreadPool ServerConfig

Server:
	gcc -c Server-files/Server.c
ThreadPool:
	gcc -c Server-files/ThreadPool.c
ServerConfig:
	gcc -c Server-files/ServerConfig.c

###############################################################################
# MARK: cJSON
###############################################################################

# Builds the cJSON library
cJSON:
	$(MAKE) -C cJSON

###############################################################################
# MARK: CLEAN
###############################################################################

# Remove all .o files
clean:
	rm -f *.o
	rm -f main
	rm -f libmini_server.a
	rm -f Server-files/*.a
	$(MAKE) -C cJSON clean
