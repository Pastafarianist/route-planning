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
    int target, length, flag;

    Edge(int target_, int length_, int flag_ = -1) : target(target_), length(length_), flag(flag_) {}
};

struct DijkstraNodeWithOrigin {
    int idx, dist, origin;

    DijkstraNodeWithOrigin(int idx_, int dist_, int origin_) : idx(idx_), dist(dist_), origin(origin_) {}
};

struct DijkstraNode {
    int idx, dist;

    DijkstraNode(int idx_, int dist_) : idx(idx_), dist(dist_) {}
};

int n, m, k;
vector<vector<Edge>> g_forw;
vector<vector<Edge>> g_back;
vector<int> part;
int total_parts;
vector<vector<bool>> flags;

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

    g_forw.resize(n);
    g_back.resize(n);
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

        g_forw[b].push_back(Edge(e, w));
        g_back[e].push_back(Edge(b, w));

        if (dir == 0) {
            g_forw[e].push_back(Edge(b, w));
            g_back[b].push_back(Edge(e, w));
        }
    }
}

void read_partition(string filename_partition) {
    assert(freopen(filename_partition.c_str(), "r", stdin));
    
    part.resize(n);

    total_parts = 0;

    forn(i, n) {
        int p;
        scanf("%d\n", &p);
        part[i] = p;
        total_parts = max(total_parts, p);
    }

    ++total_parts;
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

void preprocess() {
    vector<int> dist_back(n);
    vector<int> updated_for(n);
    vector<int> processed_for(n);

    // unordered_map does not compile here, since hash is undefined for std::pair
    map<pair<int, int>, vector<bool>> flags_mapping;

    // filling intra-part flags
    forn(s, n) {
        for (Edge& e : g_back[s]) {
            if (part[s] != part[e.target]) {
                continue;
            }
            vector<bool>& edge_flag = flags_mapping[make_pair(e.target, s)];
            if (edge_flag.size() != total_parts) {
                edge_flag.resize(total_parts);
            }
            edge_flag[part[s]] = true;
        }
    }

    auto dijkstra_cmp = [](DijkstraNodeWithOrigin a, DijkstraNodeWithOrigin b) { return a.dist > b.dist; };

    int skipped = 0;

    forn(s, n) {
        if ((n / 1000 > 0) && ((s + 1) % (n / 1000) == 0)) {
            eprintf("Done %.1f%% (processing %d/%d, skipped %d)\n", (s + 1) / (n / 100.0), s + 1, n, skipped);
        }

        // if all incoming edges are in the same part, this node can be skipped
        bool is_internal = true;
        for (Edge& inc : g_back[s]) {
            if (part[inc.target] != part[s]) {
                is_internal = false;
                break;
            }
        }
        if (is_internal) {
            dprintf("Skipping node %d because it is internal\n", s + 1);
            ++skipped;
            continue;
        }

        // Dijkstra in g_back
        int epoch = s + 1;
        priority_queue<DijkstraNodeWithOrigin, vector<DijkstraNodeWithOrigin>, function<bool(DijkstraNodeWithOrigin, DijkstraNodeWithOrigin)>> q(dijkstra_cmp);
        q.push(DijkstraNodeWithOrigin(s, 0, -1));
        dist_back[s] = 0;
        updated_for[s] = epoch;
        
        while (!q.empty()) {
            DijkstraNodeWithOrigin node = q.top();
            q.pop();

            if (processed_for[node.idx] == epoch) {
                continue;
            }
            processed_for[node.idx] = epoch;

            if (node.origin != -1) {
                vector<bool>& edge_flag = flags_mapping[make_pair(node.idx, node.origin)];
                if (edge_flag.size() != total_parts) {
                    edge_flag.resize(total_parts);
                }
                edge_flag[part[s]] = true;
            }

            for (Edge& e : g_back[node.idx]) {
                if (updated_for[e.target] != epoch || dist_back[node.idx] + e.length < dist_back[e.target]) {
                    dist_back[e.target] = dist_back[node.idx] + e.length;
                    updated_for[e.target] = epoch;
                    q.push(DijkstraNodeWithOrigin(e.target, dist_back[e.target], node.idx));
                }
            }
        }
    }

    unordered_map<vector<bool>, int> flag_index;

    for (auto& p : flags_mapping) {
        if (flag_index.find(p.second) != flag_index.end()) {
            continue;
        }
        flag_index[p.second] = flags.size();
        flags.push_back(p.second);
    }

    forn(u, n) {
        for (Edge& e : g_forw[u]) {
            e.flag = flag_index[flags_mapping[make_pair(u, e.target)]];
        }
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

    int target_part = part[t];

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
        for (Edge &e : g_forw[node.idx]) {
            vector<bool> edge_flag = flags[e.flag];
            if (edge_flag[target_part] && (updated_in[e.target] != epoch || dist[node.idx] + e.length < dist[e.target])) {
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
        assert(0 <= s && s < n);
        assert(0 <= t && t < n);
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
    if (argc != 4) {
        printf("Usage: arc_flags graph.ddsg graph.partition requests.txt\n");
        return -1;
    }

    string filename_graph = argv[1];
    string filename_partition = argv[2];
    string filename_requests = argv[3];

    timestamp("init");
    read_ddsg(filename_graph);
    timestamp("read_ddsg");
    read_partition(filename_partition);
    timestamp("read_partition");
    read_requests(filename_requests);
    timestamp("read_requests");
    preprocess();
    timestamp("preprocess");
    solve();
    timestamp("total", true);
    return 0;
}
