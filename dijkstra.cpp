#include <bits/stdc++.h>

using namespace std;

#ifdef DEBUG
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dprintf(...)
#endif

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define forn(i, n) for (int i = 0; i < n; ++i)
void timestamp(char const * const s, bool absolute = false);

struct Edge {
    int target, length;

    Edge(int target_, int length_) : target(target_), length(length_) {}
};

struct DijkstraNode {
    int idx, dist;

    DijkstraNode(int idx_, int dist_) : idx(idx_), dist(dist_) {}
};

int n, m, k;
vector<vector<Edge>> g;
vector<pair<int, int>> requests;
vector<int> dist;
vector<int> updated_in;
vector<int> processed_in;

void read_ddsg(string filename_graph) {
    assert(freopen(filename_graph.c_str(), "r", stdin));

    char c;
    scanf("%c\n", &c);
    assert(c == 'd');
    scanf("%d %d\n", &n, &m);

    g.resize(n);
    dist.resize(n);
    updated_in.resize(n);
    processed_in.resize(n);

    forn(i, m) {
        int b, e, w, dir;
        scanf("%d %d %d %d\n", &b, &e, &w, &dir);

        assert(0 <= dir && dir <= 3);

        // This is useful at least for bel.ddsg
        if (b == e) continue;

        if (dir == 3) {
            continue;
        }
        else if (dir == 2) {
            swap(b, e);
            dir = 1;
        }

        g[b].push_back(Edge(e, w));

        if (dir == 0) {
            g[e].push_back(Edge(b, w));
        }
    }
}

void read_requests(string filename_requests) {
    assert(freopen(filename_requests.c_str(), "r", stdin));

    scanf("%d\n", &k);
    forn(i, k) {
        int s, t;
        scanf("%d %d\n", &s, &t);
        requests.push_back(make_pair(s, t));
    }
}

int dijkstra(int s, int t) {
    static int epoch = 0;
    ++epoch;

    auto dijkstra_cmp = [](DijkstraNode a, DijkstraNode b) { return a.dist > b.dist; };
    priority_queue<DijkstraNode, vector<DijkstraNode>, function<bool(DijkstraNode, DijkstraNode)>> q(dijkstra_cmp);
    q.push(DijkstraNode(s, 0));
    dist[s] = 0;
    updated_in[s] = epoch;

    while (!q.empty()) {
        DijkstraNode node = q.top();
        q.pop();
        if (processed_in[node.idx] == epoch) {
            continue;
        }
        dprintf("Processing node %d\n", node.idx + 1);

        if (node.idx == t) {
            dprintf("Found path to target node %d\n", t);
            break;
        }

        processed_in[node.idx] = epoch;
        for (Edge &e : g[node.idx]) {
            if (updated_in[e.target] != epoch || dist[node.idx] + e.length < dist[e.target]) {
                dist[e.target] = dist[node.idx] + e.length;
                updated_in[e.target] = epoch;
                dprintf("Pushing node %d with distance %d\n", node.idx + 1, dist[e.target]);
                q.push(DijkstraNode(e.target, dist[e.target]));
            }
        }
    }

    if (updated_in[t] == epoch) {
        return dist[t];
    } else {
        return -1;
    }
}

void solve() {
    for (pair<int, int>& r : requests) {
        int s = r.first;
        int t = r.second;
        int d = dijkstra(s, t);
        printf("%d\n", d);
    }
    timestamp("requests processed");
}

void timestamp(char const * const s, bool absolute) {
    static double last = 0;
    double now = ((double)clock() / CLOCKS_PER_SEC) * 1000;
    if (absolute) {
        eprintf("%s: %.2lf ms\n", s, now);
    }
    else {
        eprintf("%s: %.2lf ms\n", s, now - last);
        last = now;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: dijkstra graph.ddsg requests.txt\n");
        return -1;
    }

    string filename_graph = argv[1];
    string filename_requests = argv[2];

    timestamp("init");
    read_ddsg(filename_graph);
    timestamp("read_ddsg");
    read_requests(filename_requests);
    timestamp("read_requests");
    solve();
    timestamp("total", true);
    return 0;
}
