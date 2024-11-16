###############################################################################
# MARK: ALL
###############################################################################

# Create top level static library and all sub-libraries
all: Main DataStructures Server-files


###############################################################################
# MARK: MAIN
###############################################################################

# Creates just the top level static library
Main: DataStructuresSub Server-filesSub
		ar rcs libmini_server.a Node.o LinkedList.o  BinarySearchTree.o Entry.o Dictionary.o  Server.o 



###############################################################################
# MARK: DATA STRUCTURES
###############################################################################

# Creates the data structures library
DataStructures: DataStructuresSub
	ar rcs DataStructures/DataStructures.a Node.o LinkedList.o BinarySearchTree.o Entry.o Dictionary.o
	
# Sub components of the data structures library
DataStructuresSub: Node LinkedList BinarySearchTree Entry Dictionary

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
	

###############################################################################
# MARK: NETWORKING
###############################################################################

# Creates the networking library
Server-files: Server-filesSub
	ar rcs Server-files/Server-files.a Server.o Node.o LinkedList.o BinarySearchTree.o Entry.o Dictionary.o

# Sub components of the networking library
Server-filesSub: DataStructuresSub Server

Server:
	gcc -c Server-files/Server.c
	

###############################################################################
# MARK: CLEAN
###############################################################################

# Remove all .o files
clean:
	rm *.o