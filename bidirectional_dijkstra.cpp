#include <bits/stdc++.h>
using namespace std;

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define forn(i, n) for (int i = 0; i < n; ++i)

#define INF 1000000000
#define MAXN 100000

struct Edge {
  int target, length;

  Edge(int target_, int length_) : target(target_), length(length_) {}
};

struct DijkstraNode {
  int idx, dist;

  DijkstraNode() : idx(-1), dist(INF) {}
  DijkstraNode(int idx_, int dist_) : idx(idx_), dist(dist_) {}
};

typedef priority_queue<DijkstraNode, vector<DijkstraNode>, function<bool(DijkstraNode, DijkstraNode)>> DijkstraQueue;

int n, m;
vector<vector<Edge>> g;
vector<int> dist_f;
vector<int> dist_b;
vector<int> f_updated_in;
vector<int> b_updated_in;
vector<int> f_processed_in;
vector<int> b_processed_in;

void read() {
  scanf("%d %d\n", &n, &m);

  g.resize(n);
  dist_f.resize(n);
  dist_b.resize(n);
  f_updated_in.resize(n);
  b_updated_in.resize(n);
  f_processed_in.resize(n);
  b_processed_in.resize(n);

  for (int i = 0; i < m; i++) {
    int b, e, w;
    scanf("%d %d %d", &b, &e, &w);
    --b; --e;
    g[b].push_back(Edge(e, w));
    g[e].push_back(Edge(b, w));
  }
}

int bidirectional_dijkstra(int s, int t) {
  if (s == t) {
    return 0;
  }

  static int epoch = 0;

  ++epoch;

  auto dijkstra_cmp = [] (DijkstraNode a, DijkstraNode b) { return a.dist > b.dist; };
  DijkstraQueue qf(dijkstra_cmp);
  DijkstraQueue qb(dijkstra_cmp);
  qf.push(DijkstraNode(s, 0));
  qb.push(DijkstraNode(t, 0));
  dist_f[s] = 0;
  dist_b[t] = 0;
  f_updated_in[s] = epoch;
  b_updated_in[t] = epoch;
  int mu = INF;

  while (!qf.empty() && !qb.empty()) {
    // forward
    DijkstraNode node_f = qf.top(); qf.pop();
    
    if (f_processed_in[node_f.idx] == epoch) {
      continue;
    }
    f_processed_in[node_f.idx] = epoch;

    // stopping criterion
    if (f_updated_in[node_f.idx] == epoch && 
        b_updated_in[qb.top().idx] == epoch && 
        dist_f[node_f.idx] + dist_b[qb.top().idx] >= mu) {
      break;
    }

    for (Edge& e : g[node_f.idx]) {
      // forward relaxation
      if (f_updated_in[e.target] != epoch || dist_f[node_f.idx] + e.length < dist_f[e.target]) {
        f_updated_in[e.target] = epoch;
        dist_f[e.target] = dist_f[node_f.idx] + e.length;
        qf.push(DijkstraNode(e.target, dist_f[e.target]));
      }
      // mu update
      if (b_updated_in[e.target] == epoch) {
        mu = min(mu, dist_f[node_f.idx] + e.length + dist_b[e.target]);
      }
    }

    // backward
    DijkstraNode node_b = qb.top(); qb.pop();

    if (b_processed_in[node_b.idx] == epoch) {
      continue;
    }
    b_processed_in[node_b.idx] = epoch;

    for (Edge& e : g[node_b.idx]) {
      // backward relaxation
      if (b_updated_in[e.target] != epoch || dist_b[node_b.idx] + e.length < dist_b[e.target]) {
        b_updated_in[e.target] = epoch;
        dist_b[e.target] = dist_b[node_b.idx] + e.length;
        qb.push(DijkstraNode(e.target, dist_b[e.target]));
      }
      // mu update
      if (f_updated_in[e.target] == epoch) {
        mu = min(mu, dist_b[node_b.idx] + e.length + dist_f[e.target]);
      }
    }
  }

  if (mu == INF) {
    return -1;
  }
  else {
    return mu;
  }
}

void solve() {
    int k;
    scanf("%d\n", &k);
    forn(i, k) {
        int s, t;
        scanf("%d %d\n", &s, &t);
        int d = bidirectional_dijkstra(s - 1, t - 1);
        printf("%d\n", d);
    }
}

int main() {
  string taskname = "distance";
  string fnin = taskname + ".in";
  assert(freopen(fnin.c_str(), "r", stdin));

  read();
  solve();
  eprintf("Time: %.18lf\n", (double)clock() / CLOCKS_PER_SEC);
  return 0;
}
