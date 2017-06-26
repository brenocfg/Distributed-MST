#include "algorithm.h"

enum EDGE_STATUS {
    EDGE_REJECT = -1,
    EDGE_UNKNOWN,
    EDGE_ACCEPT
};

enum MSG_TYPES {
    MSG_CONNECT = 0,
    MSG_INITIATE,
    MSG_TEST,
    MSG_ACCEPT,
    MSG_REJECT,
    MSG_CHGROOT,
    MSG_REPORT,
};

void ghs (struct node *node) {
    /*initialize all edges as UNKNOWN*/
    uint8_t edge_statuses[node->neighs->num];
    memset(edge_statuses, EDGE_UNKNOWN, sizeof(edge_statuses));

    /*we start at level 0*/
    uint32_t level = 0;

    int i;
    int min_sock = 50000, min_weight = 5000;
    struct edge *link = node->neighs->head;
    for (i = 0; i < node->neighs->num; i++) {
        if (link->weight < min_weight) {
            min_weight = link->weight;
            min_sock = link->sock;
        }
        link = link->next;
    }
    fprintf(node->log, "My lowest edge has weight %d, in sock %d!\n", min_weight, min_sock);
}
