# Distributed-MST
An implementation of the GHS algorithm, for computing a Minimum Spanning Tree of a distributed network, where nodes communicate by message passing.

Nodes in the network are represented as processes, and an edge between two nodes that can communicate with each other is represented using UNIX sockets, for inter-process communication.
