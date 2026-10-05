
Assumption:
- there is exactly one goal
- the number of slimes is less than 6.

Output constraint:
- each slime must be included to exactly one of the MST set
- sum of weight of MST is approximately minimum
- each MST must contain a goal

Output:
- a set of co-slimes



```cpp:
// One goal, few slimes (P < 6, works up to ~12): EXACT minimum.
// Grid, unit-weight edges between adjacent cells, walls blocked. Place the fewest extra slimes so that
// all slimes + the goal form one connected tree (the MST). MST weight = #edges = (#slimes not on the
// goal) + (#added slimes), so minimising added slimes minimises the MST weight.
// Method: Dreyfus-Wagner Steiner DP rooted at the goal: dp[mask][v] = fewest added cells in a tree that
// contains the slimes in `mask` and cell v. O(3^P * N^2 + 2^P * N^2 log N) -- microseconds for P < 6.
#include <bits/stdc++.h>
using namespace std;

typedef pair<int,int> Cell;              // (row, col)
typedef pair<Cell,Cell> Edge;

struct Result {
    int weight = -1;                     // MST weight, -1 if a slime cannot reach the goal
    vector<Cell> added;                  // the extra slimes to place
    vector<Edge> edges;                  // the MST (contains the goal and every slime)
};

Result solve(int N, const vector<Cell>& slimesIn, const vector<Cell>& walls, Cell goal) {
    Result res;
    int V = N * N, g = goal.first * N + goal.second;
    vector<char> wall(V, 0), slime(V, 0);
    for (auto& w : walls) wall[w.first * N + w.second] = 1;
    if (wall[g]) return res;
    vector<int> term;                                    // slime cells other than the goal
    for (auto& s : slimesIn) {
        int id = s.first * N + s.second;
        if (wall[id]) return res;                        // slime on a wall: invalid
        if (!slime[id]) { slime[id] = 1; if (id != g) term.push_back(id); }
    }
    auto cost = [&](int v) { return (slime[v] || v == g) ? 0 : 1; };   // placing a slime costs 1
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    auto neighbors = [&](int v, auto&& f) {
        for (int d = 0; d < 4; d++) {
            int x = v / N + dx[d], y = v % N + dy[d];
            if (x >= 0 && y >= 0 && x < N && y < N && !wall[x * N + y]) f(x * N + y);
        }
    };

    int T = term.size();
    vector<int> used;
    if (T > 0) {
        const int INF = 100000000;
        size_t M = (size_t)1 << T;
        vector<int> dp(M * V, INF), from(M * V, -1);     // from: (s<<1)|1 = merge split s, (u<<1) = came from u
        for (size_t mask = 1; mask < M; mask++) {
            int* cur = &dp[mask * V]; int* fr = &from[mask * V];
            if ((mask & (mask - 1)) == 0) cur[term[__builtin_ctzll(mask)]] = 0;
            else {
                size_t low = mask & (~mask + 1), rest = mask ^ low;
                for (size_t sub = (rest - 1) & rest;; sub = (sub - 1) & rest) {     // low bit always on the left
                    size_t s = sub | low;
                    for (int v = 0; v < V; v++) {
                        if (wall[v]) continue;
                        int c = dp[s * V + v] + dp[(mask ^ s) * V + v] - cost(v);
                        if (c < cur[v]) { cur[v] = c; fr[v] = (int)((s << 1) | 1); }
                    }
                    if (sub == 0) break;
                }
            }
            priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;           // grow trees along paths
            for (int v = 0; v < V; v++) if (cur[v] < INF) pq.push({cur[v], v});
            while (!pq.empty()) {
                auto [d, v] = pq.top(); pq.pop();
                if (d > cur[v]) continue;
                neighbors(v, [&](int u) {
                    int nd = d + cost(u);
                    if (nd < cur[u]) { cur[u] = nd; fr[u] = v << 1; pq.push({nd, u}); }
                });
            }
        }
        if (dp[(M - 1) * V + g] >= INF) return res;
        vector<pair<size_t,int>> st{{M - 1, g}};                                        // rebuild the tree
        while (!st.empty()) {
            auto [m, v] = st.back(); st.pop_back();
            used.push_back(v);
            int f = from[m * V + v];
            if (f < 0) continue;
            if (f & 1) { size_t s = f >> 1; st.push_back({s, v}); st.push_back({m ^ s, v}); }
            else st.push_back({m, f >> 1});
        }
    }

    vector<char> inX(V, 0);
    for (int v = 0; v < V; v++) if (slime[v]) inX[v] = 1;
    inX[g] = 1;
    for (int v : used) if (cost(v) == 1) { inX[v] = 1; res.added.push_back({v / N, v % N}); }
    sort(res.added.begin(), res.added.end());
    res.added.erase(unique(res.added.begin(), res.added.end()), res.added.end());

    vector<char> seen(V, 0);                             // spanning tree of slimes + added + goal
    vector<int> q{g}; seen[g] = 1;
    for (size_t h = 0; h < q.size(); h++)
        neighbors(q[h], [&](int u) {
            if (inX[u] && !seen[u]) { seen[u] = 1; q.push_back(u); res.edges.push_back({{q[h] / N, q[h] % N}, {u / N, u % N}}); }
        });
    res.weight = res.edges.size();
    return res;
}

int main() {
    int N, P, Q; Cell goal;
    if (scanf("%d %d %d %d %d", &N, &P, &Q, &goal.first, &goal.second) != 5) return 1;
    vector<Cell> s(P), w(Q);
    for (auto& p : s) if (scanf("%d %d", &p.first, &p.second) != 2) return 1;
    for (auto& p : w) if (scanf("%d %d", &p.first, &p.second) != 2) return 1;
    Result r = solve(N, s, w, goal);
    printf("%d\n", r.weight);                            // MST weight (-1 if impossible)
    if (r.weight < 0) return 0;
    printf("%zu\n", r.added.size());                     // number of slimes to add, then each (row col)
    for (auto& c : r.added) printf("%d %d\n", c.first, c.second);
    for (auto& e : r.edges) printf("%d %d %d %d\n", e.first.first, e.first.second, e.second.first, e.second.second);
}
```