/* EX.NO 7 - Routing Protocols
   1. Link State Routing (Dijkstra)   2. Distance Vector Routing (Bellman-Ford)
   Enter 0 in the cost matrix where there is no direct link. */
#include <stdio.h>
#define INF 9999

int n, cost[10][10];

void print_path(int *prev, int v)
{
    if (prev[v] == -1) {
        printf("%c", 'A' + v);
        return;
    }
    print_path(prev, prev[v]);
    printf(" -> %c", 'A' + v);
}

void link_state(int src)
{
    int dist[10], prev[10], vis[10], i, j, u;
    for (i = 0; i < n; i++) {
        dist[i] = INF;
        prev[i] = -1;
        vis[i] = 0;
    }
    dist[src] = 0;
    for (i = 0; i < n; i++) {
        u = -1;
        for (j = 0; j < n; j++)            /* pick nearest unvisited node */
            if (!vis[j] && (u == -1 || dist[j] < dist[u])) u = j;
        vis[u] = 1;
        for (j = 0; j < n; j++)            /* update neighbours */
            if (cost[u][j] && dist[u] + cost[u][j] < dist[j]) {
                dist[j] = dist[u] + cost[u][j];
                prev[j] = u;
            }
    }
    printf("\nLink State (Dijkstra) - shortest paths from router %c\n", 'A' + src);
    printf("Dest\tCost\tPath\n");
    for (i = 0; i < n; i++) {
        printf("%c\t%d\t", 'A' + i, dist[i]);
        print_path(prev, i);
        printf("\n");
    }
}

void distance_vector()
{
    int d[10][10], via[10][10], i, j, k, changed = 1, round = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) {
            d[i][j] = (i == j) ? 0 : (cost[i][j] ? cost[i][j] : INF);
            via[i][j] = (cost[i][j] || i == j) ? j : -1;
        }
    while (changed) {                      /* routers exchange tables until stable */
        changed = 0;
        round++;
        for (i = 0; i < n; i++)
            for (k = 0; k < n; k++)
                if (cost[i][k])
                    for (j = 0; j < n; j++)
                        if (cost[i][k] + d[k][j] < d[i][j]) {
                            d[i][j] = cost[i][k] + d[k][j];
                            via[i][j] = k;
                            changed = 1;
                        }
    }
    printf("\nDistance Vector (Bellman-Ford) - converged after %d rounds\n", round);
    for (i = 0; i < n; i++) {
        printf("\nRouting table of router %c\nDest\tNext hop\tCost\n", 'A' + i);
        for (j = 0; j < n; j++)
            printf("%c\t%c\t\t%d\n", 'A' + j, 'A' + via[i][j], d[i][j]);
    }
}

int main()
{
    int i, j, ch;
    char src;
    printf("1.Link State Routing  2.Distance Vector Routing\nEnter choice: ");
    scanf("%d", &ch);
    printf("Enter number of routers: ");
    scanf("%d", &n);
    printf("Enter cost matrix (0 = no link):\n");
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            scanf("%d", &cost[i][j]);
    if (ch == 1) {
        printf("Enter source router (A, B, C...): ");
        scanf(" %c", &src);
        link_state(src - 'A');
    } else {
        distance_vector();
    }
    printf("\nRESULT: Routing tables computed successfully\n");
    return 0;
}
