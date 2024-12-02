###############################################################################
# MARK: ALL
###############################################################################

# Create top level static library and all sub-libraries
all: Main DataStructures Server-files main_exec


###############################################################################
# MARK: EXECUTABLE
###############################################################################

# Build the main executable
main_exec: Main
	gcc -o main main.c -L. -lmini_server

###############################################################################
# MARK: MAIN
###############################################################################

# Creates just the top level static library
Main: DataStructuresSub Server-filesSub
		ar rcs libmini_server.a Node.o LinkedList.o  BinarySearchTree.o Entry.o Dictionary.o Queue.o Server.o HTTPRequest.o ThreadPool.o



###############################################################################
# MARK: DATA STRUCTURES
###############################################################################

# Creates the data structures library
DataStructures: DataStructuresSub
	ar rcs DataStructures/DataStructures.a Node.o LinkedList.o BinarySearchTree.o Entry.o Dictionary.o Queue.o
	
# Sub components of the data structures library
DataStructuresSub: Node LinkedList BinarySearchTree Entry Dictionary Queue

Node:
	gcc -c DataStructures/Common/Node.c

LinkedList:
	gcc -c DataStructures/Lists/LinkedList.c
	
BinarySearchTree:
	gcc -c DataStructures/Trees/BinarySearchTree.c

Entry:
	gcc -c DataStructures/Dictionary/Entry.c

Dictionary:
	gcc -c DataStructures/Dictionary/Dictionary.c

Queue:
	gcc -c DataStructures/Lists/Queue.c
	

###############################################################################
# MARK: NETWORKING
###############################################################################

# Creates the networking library
Server-files: Server-filesSub
	ar rcs Server-files/Server-files.a Server.o Node.o LinkedList.o BinarySearchTree.o Entry.o Dictionary.o HTTPRequest.o ThreadPool.o

# Sub components of the networking library
Server-filesSub: DataStructuresSub Server HTTP ThreadPool

Server:
	gcc -c Server-files/Server.c
HTTP:
	gcc -c Server-files/HTTPRequest.c
ThreadPool:
	gcc -c Server-files/ThreadPool.c

###############################################################################
# MARK: CLEAN
###############################################################################

# Remove all .o files
clean:
	rm *.o