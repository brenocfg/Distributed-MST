# Distributed-MST #

An implementation of the GHS algorithm, for computing a Minimum Spanning Tree of
a distributed network, where nodes communicate by message passing.

In this implementation, nodes in the network are represented by processes in the
operating system. For each edge in the graph, the two adjacent nodes keep a pair
of UNIX sockets, created via socketpair(), to represent that edge. The nodes
then communicate through the usual send()/recv() interface for sockets.

# Folder structure #

A brief description of each implementation file and what they contain:

* main.c - Implements the parent process, which parses input parameters,
computes network topology, forks child processes for node execution, then waits
for them to finish before terminating.
* msgqueue.c - Implements an atomic generic message queue, which is used as a
node's queue of messages from where it receives data from all its neighbours.
Message queues are atomic so as to keep them consistent during multi-threaded
access.
* neighlist.c - Implements a given node's list of neighbours, which is
essentially a linked list of edges, with each edge having an associated weight
and socket.
* node.c - Implements a generic node structure. Nodes are minimal and supposed
to be algorithm-agnostic, so the only things the node structure itself maintains
are the node's ID, its list of neighbours and its message queue, and streams for
its local log file and the global distributed log. Any algorithm-specific
information should be kept in the algorithm's implementation itself.
* algorithm - Implements the algorithm that each node will run after
initialization. In this case the algorithm is the GHS algorithm for computing
Distributed Minimum Spanning Trees, but the underlying structure of a network
of nodes is supposed to be generic, in case we need to implement other
algorithms later on :).

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
0/blank), the program will generate a dense graph, with a LOT of edges (namely
n-1*n-2/2+1 edges) to guarantee that the graph is connected. Otherwise, the program
will connect each consecutive pair of nodes with each other, to guarantee a
connected graph, then generate a small additional number of edges to add variety
to the network topology.

# Functionality #

The program functions by first computing a network topology, with the specified
number of nodes. Then it forks a child process for each node, who then proceeds
to initialize its local data (ID, msg queue, log file, etc.). At this point the
nodes no longer access any global data from the parent process, aside from the
global shared log. For its edges, a given node will spawn one thread for each
of the sockets that represent an edge, and each thread will listen for messages
on the given socket, adding them to the node's aggregate message queue. The
message queue structure was specifically implemented with all operations being
atomic, so all threads can access it without corrupting its state or causing
race conditions.

Each node then moves on to run the assigned algorithm function (in this case,
GHS). Each node creates a local log file with filename "x.log", where x is its
ID, to log internal events, such as sending/receiving messages, state changes
and whatnot. Nodes also log 'important' events in the global distributed log,
such as beginning of execution, algorithm termination, etc. Therefore, if one
wants the big picture of an algorithm execution, reading through the global.log
file should be enough. For a more detailed breakdown of each node's behaviour
throughout execution, refer to that node's local log.

# Asynchrony #

Since it is 2017 and computers are really fast, simply forking child processes
and letting them all execute happily ever after is not enough to simulate a
truly asynchronous network, since nodes would execute, as a wise man once said,
"as fast as lightning", almost as if they were kung-fu fighting. Therefore, a
couple of tweaks were implemented to force a small amount of asynchrony in the
network (though probably not the ideal amount):

* The parent thread for each node sleeps for a random amount of time, between 0
and ~4 seconds, before beginning node execution, to stop all nodes from waking
up simultaneously.
* Whenever a node needs to delay its response to a message, usually because the
sender of the message is in a higher-level fragment than the node itself, the
node will sleep for a second after moving the message to the back of its queue.
This was honestly not as much implemented for asynchrony, but more to stop nodes
from spamming their logs with thousands of redundant log messages, but hey, you
won't tell if I don't, right?
* Finally, to simulate varying latency for links in a real network, each thread
responsible for listening for messages in a given socket will sleep for a random
amount of time between 0 and ~4 seconds before actually adding the message to
the node's queue.

# The GHS implementation #

The implementation of the GHS algorithm itself is all contained in the
algorithm.c file.

Each node begins by running the 'wakeup' procedure specified in the algorithm.
This means that technically we 'wake up' every node at the beginning of
execution, rather than waiting for other external events to wake them up.
During wake up, each node initially assigns itself to level 0, and proceed to
identify its lowest weight edge, sending a CONNECT message across it with the
level 0 identifier. This essentially will trigger a MERGE event, for a given
pair of nodes who choose the same edge as their lowest weight one. During this
stage, each node sends a single message, so total number of messages sent is
n -> O(n).

All nodes will then essentially move onto an infinite "reactive" loop, where
they simply await for messages to arrive in its message queue, then pop the
first message in the queue, and react appropriately based on its type and
content.

After wake up, both of the newly democratically elected (but not really) nodes
will send INITIATE messages to each other, moving them to the next level.
At this point, nodes from lower levels which have not created their own
prospering communities yet can only be absorbed by the recently thriving
civilized nodes, by receiving INITIATE messages as reply to its CONNECT
attempts. For every step in level advancement in the algorithm, each node will
receive at most a single INITIATE message, so therefore another at most n
-> O(n) number of messages is sent.

After the CONNECT/INITIATE phase, each node will move on to the discovery phase,
where they attempt to compute its Lowest Weight Outgoing Edge (LWOE), which is
the least costly of its edges that leads to a node that is outside the current
node's fragment (or community, as nodes prefer it). To do this, each node sends
at most one TEST message across each of its edges, and can receive at most a
single ACCEPT message through one of them, before it determines its LWOE and
stops. If an edge ends up being rejected through a node sending a REJECT message
across it, that edge is never tested again. Therefore, throughout all of the
discovery phases in the algorithm, for rejected edges the number of messages is
at most 2E (TEST + REJECT). For each accepted edge, a node sends a single TEST
message, receives at most a single ACCEPT message, and then sends at most a
single REPORT message, plus a CONNECT or CHGCORE message, so we have, for each
discovery phase, at most up to 5N -> O(n) messages sent for each successful LWOE
search.

After all nodes compute their LWOE, they report the weight of said edge to their
'parent's in the MST, by sending a REPORT message through the edge from which
they initially received the INITIATE message for their current level (cough
community cough). These messages travel up the BRANCH edges all the way up to
the two nodes who were originally the leaders of said community. These nodes
will then send either a **single** CHGROOT message (in case they are not a
leader in the next upcoming generation), or a **single** CONNECT message, both
of which we've accounted for in the last paragraph.

Finally, for this specific implementation, the student in question was not
competent enough to have the algorithm terminate as originally specified in the
paper. Each node is supposed to terminate organically after they all realize
that the utopian all-encompassing community has been built, and peacefully go
back to sleep. However, in this case, for some reason, only the two community
leader nodes were terminating, with all other nodes awaiting for additional
information. Therefore, an extra broadcast phase was implemented, where before
terminating the leader nodes broadcast a REPORT message through its BRANCH
edges, essentially signaling all other nodes to terminate as well. This adds an
extra n -> O(n) messages to the mix, one for each node in the network.

Finally, through the proofs contained in the paper, we know that the algorithm
goes through at most logn levels before terminating. Therefore, we have a total
communication cost of: 2E + 6N*logn messages, which should be O(nlogn) in
overall complexity. But hey, good communication is the staple of a stable
relationship (or community, I guess).
