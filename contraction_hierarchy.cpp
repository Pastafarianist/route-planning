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

const int INF = 1000000000;

struct Edge {
    int target, length;
    int through;

    Edge() = default;

    Edge(int target_, int length_, int through_ = -1) :
            target(target_), length(length_), through(through_) {}
};

int n, m, k;
vector<unordered_map<int, Edge>> g_forw;
vector<unordered_map<int, Edge>> g_back;
vector<pair<int, int>> requests;

vector<int> level;

struct DijkstraNode {
    int idx, dist;

    DijkstraNode(int idx_, int dist_) : idx(idx_), dist(dist_) {}
};

typedef priority_queue<DijkstraNode, vector<DijkstraNode>, function<bool(DijkstraNode, DijkstraNode)>> DijkstraQueue;

vector<int> dist_forw;
vector<int> dist_back;
vector<int> forw_updated_in;
vector<int> back_updated_in;
vector<int> forw_processed_in;
vector<int> back_processed_in;
int epoch = 0;

void allocate() {
    g_forw.resize(n);
    g_back.resize(n);

    level.resize(n);

    dist_forw.resize(n);
    dist_back.resize(n);
    forw_updated_in.resize(n);
    back_updated_in.resize(n);
    forw_processed_in.resize(n);
    back_processed_in.resize(n);
}

void read_ddsg(string filename_graph) {
    assert(freopen(filename_graph.c_str(), "r", stdin));

    char c;
    scanf("%c\n", &c);
    assert(c == 'd');
    scanf("%d %d\n", &n, &m);

    allocate();

    for (int i = 0; i < m; i++) {
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

        // The following two assertions don't hold for bel.ddsg
//        assert(g_forw[b].find(e) == g_forw[b].end() || g_forw[b][e].length == w);
//        assert(g_back[e].find(b) == g_back[e].end() || g_back[e][b].length == w);

        if (g_forw[b].find(e) == g_forw[b].end() || g_forw[b][e].length > w) g_forw[b][e] = Edge(e, w);
        if (g_back[e].find(b) == g_back[e].end() || g_back[e][b].length > w) g_back[e][b] = Edge(b, w);

        if (dir == 0) {
            if (g_forw[e].find(b) == g_forw[e].end() || g_forw[e][b].length > w) g_forw[e][b] = Edge(b, w);
            if (g_back[b].find(e) == g_back[b].end() || g_back[b][e].length > w) g_back[b][e] = Edge(e, w);
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

struct NodeWithDegree {
    int node, degree;

    NodeWithDegree(int node_, int degree_) : node(node_), degree(degree_) {}
};

struct NWD_compare {
    bool operator() (const NodeWithDegree& lhs, const NodeWithDegree& rhs) {
        if (lhs.degree != rhs.degree) {
            return lhs.degree < rhs.degree;
        }
        return lhs.node < rhs.node;
    }
};

void build_hierarchy() {
    set<NodeWithDegree, NWD_compare> importance;
    vector<int> node2importance;

    vector<int> g_forw_uncontracted;
    vector<int> g_back_uncontracted;
    vector<bool> contracted;

    node2importance.resize(n);
    g_forw_uncontracted.resize(n);
    g_back_uncontracted.resize(n);
    contracted.resize(n);

    forn(i, n) {
        g_forw_uncontracted[i] = g_forw[i].size();
        g_back_uncontracted[i] = g_back[i].size();
        int imp = g_forw_uncontracted[i] + g_back_uncontracted[i];
        importance.insert(NodeWithDegree(i, imp));
        node2importance[i] = imp;
    }

    level.assign(n, -1);

    int total_shortcuts = 0;

    forn(i, n) {
        assert(importance.size() == n - i);
        NodeWithDegree nwd = *importance.begin();
        importance.erase(importance.begin());
        int node = nwd.node;
        assert(node2importance[node] == nwd.degree);

        dprintf("Contracting node %d with degree=%d\n", node + 1, nwd.degree);

        // add shortcuts
        for (pair<const int, Edge> &p_inc : g_back[node]) {
            Edge &inc = p_inc.second;
            if (contracted[inc.target]) continue;
            for (pair<const int, Edge> &p_out : g_forw[node]) {
                Edge &out = p_out.second;
                if (contracted[out.target]) continue;

                // looking at path from `inc.target` to `out.target`

                if (inc.target == out.target) {
                    continue;
                }

                int shortcut_length = inc.length + out.length;

                bool will_replace = false;

                if (g_forw[inc.target].find(out.target) != g_forw[inc.target].end()) {
                    // This edge is already in the graph
                    will_replace = true;
                    if (g_forw[inc.target].at(out.target).length <= shortcut_length) {
                        // and it is at least as optimal
                        continue;
                    }
                }

                Edge forw_shortcut = Edge(out.target, shortcut_length, node);
                Edge back_shortcut = Edge(inc.target, shortcut_length, node);

                if (will_replace) {
                    dprintf("    Replacing edge [%d→%d]:%d with a shortcut:%d through %d\n", inc.target + 1, out.target + 1,
                            g_forw[inc.target].at(out.target).length, shortcut_length, node + 1);
                } else {
                    dprintf("    Adding shortcut [%d→%d]:%d through %d\n", inc.target + 1, out.target + 1, shortcut_length, node + 1);
                }

                if (!will_replace) {
                    ++total_shortcuts;

                    ++g_forw_uncontracted[inc.target];
                    ++g_back_uncontracted[out.target];
                }

                g_forw[inc.target][out.target] = forw_shortcut;
                g_back[out.target][inc.target] = back_shortcut;
            }
        }

        contracted[node] = true;

        // calculate level for contracted node
        int node_level = 0;
        for (pair<const int, Edge> &p_inc : g_back[node]) {
            Edge &inc = p_inc.second;
            node_level = max(node_level, level[inc.target] + 1);
        }
        for (pair<const int, Edge> &p_out : g_forw[node]) {
            Edge &out = p_out.second;
            node_level = max(node_level, level[out.target] + 1);
        }
        level[node] = node_level;

        dprintf("    Node %d level is %d\n", node + 1, node_level);

        // update importance
        for (pair<const int, Edge> &p_inc : g_back[node]) {
            Edge &inc = p_inc.second;
            if (contracted[inc.target]) continue;

            assert(--g_forw_uncontracted[inc.target] >= 0);

            importance.erase(NodeWithDegree(inc.target, node2importance[inc.target]));
            int imp = g_forw_uncontracted[inc.target] + g_back_uncontracted[inc.target];
            importance.insert(NodeWithDegree(inc.target, imp));
            node2importance[inc.target] = imp;
        }

        for (pair<const int, Edge> &p_out : g_forw[node]) {
            Edge &out = p_out.second;
            if (contracted[out.target]) continue;

            assert(--g_back_uncontracted[out.target] >= 0);

            importance.erase(NodeWithDegree(out.target, node2importance[out.target]));
            int imp = g_forw_uncontracted[out.target] + g_back_uncontracted[out.target];
            importance.insert(NodeWithDegree(out.target, imp));
            node2importance[out.target] = imp;
        }
    }

    dprintf("Final state:\n");
    forn(i, n) {
        dprintf("    Node %d has level %d\n", i + 1, level[i]);
        dprintf("    Outgoing edges at %d:\n", i + 1);
        for (pair<const int, Edge> &p_out : g_forw[i]) {
            dprintf("        %d -> %d: %d", i + 1, p_out.second.target + 1, p_out.second.length);
            if (p_out.second.through == -1) {
                dprintf("\n");

            } else {
                dprintf(", through %d\n", p_out.second.through + 1);
            }
        }
        dprintf("    Incoming edges at %d:\n", i + 1);
        for (pair<const int, Edge> &p_inc : g_back[i]) {
            dprintf("        %d -> %d: %d", p_inc.second.target + 1, i + 1, p_inc.second.length);
            if (p_inc.second.through == -1) {
                dprintf("\n");

            } else {
                dprintf(", through %d\n", p_inc.second.through + 1);
            }
        }
    }
    eprintf("Total shortcuts: %d\n", total_shortcuts);
}

int query_hierarchy(int s, int t) {
    if (s == t) {
        return 0;
    }

    dprintf("Calculating distance from %d to %d\n", s + 1, t + 1);

    ++epoch;

    auto dijkstra_cmp = [](DijkstraNode a, DijkstraNode b) { return a.dist > b.dist; };
    DijkstraQueue q_forw(dijkstra_cmp);
    DijkstraQueue q_back(dijkstra_cmp);
    q_forw.push(DijkstraNode(s, 0));
    q_back.push(DijkstraNode(t, 0));
    dist_forw[s] = 0;
    forw_updated_in[s] = epoch;
    dist_back[t] = 0;
    back_updated_in[t] = epoch;
    int mu = INF;

    bool forw_aborted = false;
    bool back_aborted = false;

    while ((!forw_aborted && !q_forw.empty()) || (!back_aborted && !q_back.empty())) {
        // forward
        if (!forw_aborted && !q_forw.empty()) {
            DijkstraNode node_f = q_forw.top();
            q_forw.pop();

            if (forw_processed_in[node_f.idx] == epoch) {
                continue;
            }
            forw_processed_in[node_f.idx] = epoch;

            dprintf("    Forward:  processing node %d [q_forw.size() == %ld]\n", node_f.idx + 1, q_forw.size());

            // stopping criterion
            // note: default bidirectional dijkstra stopping criterion is not applicable here,
            // since this isn't a search in g and g^op, this is actually a search in two distinct graphs
            if (dist_forw[node_f.idx] >= mu) {
                dprintf("    Aborting forward search:\n");
                dprintf("        node_f index is %d\n", node_f.idx + 1);
                dprintf("        dist_forw[node_f.idx] == %d\n", dist_forw[node_f.idx]);
                dprintf("        mu == %d\n", mu);
                forw_aborted = true;
            }

            if (!forw_aborted) {
                for (pair<const int, Edge> &p : g_forw[node_f.idx]) {
                    Edge &e = p.second;
                    // only going up the hierarchy
                    if (level[e.target] <= level[node_f.idx]) {
                        continue;
                    }
                    // forward relaxation
                    if (forw_updated_in[e.target] != epoch || dist_forw[node_f.idx] + e.length < dist_forw[e.target]) {
                        dist_forw[e.target] = dist_forw[node_f.idx] + e.length;
                        forw_updated_in[e.target] = epoch;
                        dprintf("        Pushing node %d with distance %d\n", e.target + 1, dist_forw[e.target]);
                        q_forw.push(DijkstraNode(e.target, dist_forw[e.target]));
                    }
                    // mu update
                    if (back_updated_in[e.target] == epoch) {
                        mu = min(mu, dist_forw[node_f.idx] + e.length + dist_back[e.target]);
                    }
                }

            }
        }

        // backward
        if (!back_aborted && !q_back.empty()) {
            DijkstraNode node_b = q_back.top();
            q_back.pop();

            if (back_processed_in[node_b.idx] == epoch) {
                continue;
            }
            back_processed_in[node_b.idx] = epoch;

            dprintf("    Backward: processing node %d [q_back.size() == %ld]\n", node_b.idx + 1, q_back.size());

            // stopping criterion
            if (dist_back[node_b.idx] >= mu) {
                dprintf("    Aborting backward search:\n");
                dprintf("        node_b index is %d\n", node_b.idx + 1);
                dprintf("        dist_back[node_b.idx] == %d\n", dist_back[node_b.idx]);
                dprintf("        mu == %d\n", mu);
                back_aborted = true;
            }

            if (!back_aborted) {
                for (pair<const int, Edge> &p : g_back[node_b.idx]) {
                    Edge &e = p.second;
                    // only going up the hierarchy
                    if (level[e.target] <= level[node_b.idx]) {
                        continue;
                    }
                    // backward relaxation
                    if (back_updated_in[e.target] != epoch || dist_back[node_b.idx] + e.length < dist_back[e.target]) {
                        dist_back[e.target] = dist_back[node_b.idx] + e.length;
                        back_updated_in[e.target] = epoch;
                        dprintf("        Pushing node %d with distance %d\n", e.target + 1, dist_back[e.target]);
                        q_back.push(DijkstraNode(e.target, dist_back[e.target]));
                    }
                    // mu update
                    if (forw_updated_in[e.target] == epoch) {
                        mu = min(mu, dist_back[node_b.idx] + e.length + dist_forw[e.target]);
                    }
                }
            }
        }
    }

    if (mu != INF) {
        dprintf("    Distance from %d to %d is %d\n", s + 1, t + 1, mu);
        return mu;
    }

    dprintf("    Could not find a path from %d to %d\n", s + 1, t + 1);
    return -1;
}

void solve() {
    build_hierarchy();
    timestamp("build_hierarchy");

    for (pair<int, int>& r : requests) {
        int s = r.first;
        int t = r.second;
        int d = query_hierarchy(s, t);
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
        printf("Usage: contraction_hierarchy graph.ddsg requests.txt\n");
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
