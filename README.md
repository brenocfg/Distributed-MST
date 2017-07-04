# Distributed-MST #
An implementation of the GHS algorithm, for computing a Minimum Spanning Tree of
a distributed network, where nodes communicate by message passing.

In this implementation, nodes in the network are represented by processes in the
operating system. For each edge in the graph, the two adjacent nodes keep a pair
of UNIX sockets, created via socketpair(), to represent that edge. The nodes
then communicate through the usual send()/recv() interface for sockets.

# Building #

The program can be built by simply running 'make' in the repository folder.
The generated binary will be named 'ghs'.

NOTE: This will only work in UNIX systems, since we use the (technically
deprecated) usleep() function for sleeping with millisecond precision.

# Running #

The syntax for running the program is as follows:

    ghs <number of nodes [15-100]> <density flag>

Where number of nodes specifies the number of nodes to be created, which needs
to be between 15 and 100. If density flag is set to 1 (or any value but
0/blank), the program will generate a dense graph, with a LOT
