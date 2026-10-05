### prompt:

> A deviation is about shifting a pair of arbitrary junctions to merge a path (a set of continuous edges) of the same direction from another dMST. Do not search for deviations between dMSTs directly. Decompose every dMST into maximal directed straight segments, then exhaustively compare segment pairs across different dMSTs. For each pair, compute their relative displacement vector, parallel/perpendicular distance, projected overlap, and endpoint gap. Treat nearby parallel segments and nearby endpoints as deviation candidates, even when the two dMSTs have no common points. Rank candidates by estimated shared travel gained versus deviation cost, and only run the expensive state/collision simulation on the highest-ranked candidates. 


### outcome:

**Model I'm assuming:**
- Each dMST is a set of directed edges between adjacent cells, forming an in-tree toward its root (the goal). Its leaves are the slimes that must stay connected.
- **Score** = number of distinct directed edges in the union of all dMSTs. A shared edge is paid once, which is the "join" (being carried).
- **Scaffolding** is landing on a cell of another dMST.
- **Deviation** = replace one straight segment of dMST A with a detour: a connector into a straight segment of dMST B, riding B's edges, then a connector back to A. A's own length may go up as long as the global score goes down.
- **Collision** = two dMSTs using the same edge in opposite directions.
- **Pipeline**: decompose into straight segments, compare all cross-tree segment pairs (displacement, parallel and perpendicular distance, overlap, endpoint gap), rank by estimated gain, and run the exact simulation only on the top M. If nothing improves, a depth-2 lookahead tolerates a locally worse first deviation.That's much better: 79% of the gap closed, with the result on average 7.5% above the true optimum. I'll check whether a wider candidate set or a larger top-M buys more.That timed out. The bottleneck was recomputing the exact optimum for every configuration. I'll cache it, use a smaller instance set, and bound each run.I built `dmst_deviation.cpp`, which follows your pipeline and is verified on random instances at your limits. On average it lands about 7% above the true optimum and runs in under 15 ms. The statement never defines the score or the simulation, so I fixed a concrete model (listed below).

**Pipeline, as you described it:**
1. Decompose every dMST into maximal directed straight segments, broken at turns, merge junctions and leaves.
2. Compare all cross-dMST segment pairs: relative displacement, parallel and perpendicular distance, projected overlap, and endpoint gap. Keep nearby parallel segments and nearby endpoints, even when the dMSTs share no cell.
3. Rank candidates by estimated shared length gained minus connector cost, and run the exact simulation only on the top 40.
4. Apply the best improving deviation and repeat. If none improves, look two deviations ahead, so a locally worse first deviation can pay off. That is your "tolerate local increase for the global minimum". It fired in 14 of 308 test instances.

**The model I fixed (tell me where it differs):**
- The score is the number of distinct directed edges across all dMSTs, so a shared edge is paid once (join).
- Scaffolding is landing on a cell of another dMST.
- A deviation cuts the path from a segment start `a0` to a downstream cell `q`. It rebuilds the path as a connector to another dMST's segment, then a ride along its edges, then a connector to `q`. Choosing `q` as the goal means "carried all the way". Without that option the search closed only 22% of the gap to optimal, versus 79% with it.
- The simulation rejects a move if any slime can no longer reach the goal, if there is a cycle, or if two dMSTs use the same edge in opposite directions.
- Leaves are the slimes that must stay connected, and the node with no out-edge is the root.

**Results** on 308 random instances (2–6 dMSTs, at most 20 edges each, at most 40 walls, grid up to 20×20):
- Every output was checked independently: valid in-trees, no walls, no head-on collisions, and the reported score matches my own recount.
- The score dropped in 304 of 308 instances, from an average of 37.6 to 29.5.
- The true optimum under this model, computed exactly with the earlier Steiner solver, is 27.2. Results average 7.5% above it (28% at worst).
- A wider search with 4× the work only moved the gap closed from 79% to 81%, so I kept the defaults.

**Usage:**
```
./dmst [M=40] [cap=0] [gap=6] [perp=6] < input
input:   N W K / W wall coords / for each dMST: E, then E lines "r1 c1 r2 c2" (edge u->v toward the root)
output:  score <before> <after> / deviations n / DEV ... lines / TREE i E + its edges
```
`cap` limits edges per dMST in the output. I treated your 20-edge bound as an input limit only, so it is off by default.

This model has an important consequence: because sharing is unlimited, the true optimum is one shared Steiner tree, and it can be computed exactly for up to about 13 slimes. If your real problem has limits that make the deviation search necessary, that is probably where "scaffolding" comes in. What are the exact rules for a jump, such as capacity, direction, or timing? Once I know them I'll replace the scoring and simulation functions, and the rest of the pipeline stays as it is.

```cpp:
// Deviation search over several directed MSTs (dMSTs) on an N x N grid with walls.
//
// Model (see the notes in the reply):
//   * a dMST = set of directed edges between adjacent cells, an in-tree toward its root; leaves are the slimes.
//   * score  = number of DISTINCT directed edges over all dMSTs (an edge used by several dMSTs is paid once:
//              that is "join"; landing on another dMST's cell is "scaffolding").
//   * collision = an edge used in opposite directions by any two dMSTs.
//   * deviation = replace the path a0 -> q of dMST A (a0 = start of one of its straight segments, q = that segment's
//              end or any cell further downstream) by  a0 ~> x (connector),  x -> y (ride a straight segment of
//              another dMST B),  y ~> q (connector).  q = the goal means "be carried all the way".
//              A's own length may grow if the global score falls.
// Pipeline:
//   1. decompose every dMST into maximal directed straight segments (broken at turns / merge junctions / leaves)
//   2. compare all cross-dMST segment pairs: displacement, parallel + perpendicular distance, projected overlap,
//      endpoint gap;  keep nearby parallel segments and nearby endpoints (even if the dMSTs share no cell)
//   3. rank by estimated gain (shared length gained - connector cost), simulate only the top M exactly
//   4. apply the best improving deviation, repeat; if none improves, look 2 deviations deep so a locally worse
//      first deviation can pay off (the "tolerated deviance").
#include <bits/stdc++.h>
using namespace std;

static int N, V;
static vector<char> wallv;
static vector<vector<int>> D;                       // hop distance around walls, -1 = unreachable
static const int DR[4] = {-1, 0, 1, 0}, DC[4] = {0, 1, 0, -1};   // 0 up, 1 right, 2 down, 3 left; opposite = d^2

static int dirOf(int u, int v) {
    int dr = v / N - u / N, dc = v % N - u % N;
    for (int d = 0; d < 4; d++) if (DR[d] == dr && DC[d] == dc) return d;
    return -1;
}
static int stepTo(int u, int d) { return (u / N + DR[d]) * N + (u % N + DC[d]); }
static int manh(int u, int v) { return abs(u / N - v / N) + abs(u % N - v % N); }

struct Tree { vector<int> nxt; int root = -1; vector<int> src; };     // nxt[v] = head of v's out-edge, -1 none
typedef vector<Tree> Config;

static vector<int> unionCount(const Config& C) {
    vector<int> cnt(V * 4, 0);
    for (auto& t : C) for (int v = 0; v < V; v++) if (t.nxt[v] != -1) cnt[v * 4 + dirOf(v, t.nxt[v])]++;
    return cnt;
}
// exact "state / collision simulation": false on a head-on collision, else the score
static bool evaluate(const Config& C, int& score) {
    vector<int> cnt = unionCount(C);
    score = 0;
    for (int v = 0; v < V; v++) for (int d = 0; d < 4; d++) if (cnt[v * 4 + d] > 0) {
        score++;
        if (cnt[stepTo(v, d) * 4 + (d ^ 2)] > 0) return false;
    }
    return true;
}

struct Seg { int tree, dir; vector<int> nodes; };
static vector<Seg> segmentsOf(const Config& C) {
    vector<Seg> out;
    for (int t = 0; t < (int)C.size(); t++) {
        const Tree& T = C[t];
        vector<int> indeg(V, 0), pred(V, -1);
        for (int v = 0; v < V; v++) if (T.nxt[v] != -1) { indeg[T.nxt[v]]++; pred[T.nxt[v]] = v; }
        for (int v = 0; v < V; v++) {
            if (T.nxt[v] == -1) continue;
            int d = dirOf(v, T.nxt[v]);
            bool cont = indeg[v] == 1 && dirOf(pred[v], v) == d;      // v is the middle of a straight run
            if (cont) continue;
            Seg s{t, d, {v}};
            int cur = v;
            while (true) {
                int n = T.nxt[cur];
                s.nodes.push_back(n);
                if (indeg[n] == 1 && T.nxt[n] != -1 && dirOf(n, T.nxt[n]) == d) cur = n; else break;
            }
            out.push_back(s);
        }
    }
    return out;
}

struct Move { int ta, tb, q, est, shared, perp; vector<int> a, b; };   // a = replaced segment, b = ridden run, q = reconnect cell in A
struct Params { int M = 40, L = 12, maxPerp = 6, maxGap = 6, cap = 0; };

static vector<Move> generate(const Config& C, const Params& P) {
    vector<int> cnt = unionCount(C);
    vector<Seg> segs = segmentsOf(C);
    vector<vector<int>> indeg(C.size(), vector<int>(V, 0));
    for (size_t t = 0; t < C.size(); t++) for (int v = 0; v < V; v++) if (C[t].nxt[v] != -1) indeg[t][C[t].nxt[v]]++;
    vector<Move> out;
    for (auto& a : segs) {
        const Tree& TA = C[a.tree];
        int a0 = a.nodes.front(), a1 = a.nodes.back(), la = a.nodes.size() - 1;
        // A's downstream chain from a0 to the root, and how many edges each reconnect point would free
        vector<int> chain;
        for (int v = a0;; v = TA.nxt[v]) { chain.push_back(v); if (v == TA.root) break; }
        vector<int> freed(chain.size(), 0);
        { int fr = 0; bool stop = false;
          for (size_t e = 0; e + 1 < chain.size(); e++) {
              if (!stop) {
                  if (e >= 1 && indeg[a.tree][chain[e]] >= 2) stop = true;             // another branch still needs it
                  else if (cnt[chain[e] * 4 + dirOf(chain[e], chain[e + 1])] == 1) fr++;   // only A pays for this edge
              }
              freed[e + 1] = fr;
          } }
        for (auto& b : segs) {
            if (a.tree == b.tree) continue;
            int b0 = b.nodes.front(), b1 = b.nodes.back(), lb = b.nodes.size() - 1;
            // ---- pair features ----
            int gap = min({manh(a0, b0), manh(a0, b1), manh(a1, b0), manh(a1, b1)});     // endpoint gap
            int perp = INT_MAX, overlap = 0;
            bool same = a.dir == b.dir;
            if (same) {
                int dr = b0 / N - a0 / N, dc = b0 % N - a0 % N;                          // relative displacement b0 - a0
                int par = dr * DR[a.dir] + dc * DC[a.dir];                               // parallel component
                perp = abs(dr - par * DR[a.dir]) + abs(dc - par * DC[a.dir]);            // perpendicular distance
                overlap = max(0, min(la, par + lb) - max(0, par));                       // projected overlap
            }
            bool cand = (same && perp <= P.maxPerp && (overlap > 0 || gap <= P.maxGap)) || gap <= P.maxGap;
            if (!cand) continue;
            // ---- estimated gain: best (ride x..y on b, reconnect at chain[qi]) ----
            int bestE = INT_MAX, bi = -1, bj = -1, bq = -1;
            for (int i = 0; i <= lb; i++) {
                int d1 = D[a0][b.nodes[i]];
                if (d1 < 0) continue;
                for (int j = i + 1; j <= lb; j++) for (int qi = la; qi < (int)chain.size(); qi++) {
                    int d2 = D[b.nodes[j]][chain[qi]];
                    if (d2 < 0) continue;
                    int e = d1 + d2 - freed[qi];
                    if (e < bestE || (e == bestE && j - i > bj - bi)) { bestE = e; bi = i; bj = j; bq = chain[qi]; }
                }
            }
            if (bi < 0) continue;
            Move m; m.ta = a.tree; m.tb = b.tree; m.q = bq; m.est = bestE; m.shared = bj - bi; m.perp = same ? perp : 99;
            m.a = a.nodes; m.b.assign(b.nodes.begin() + bi, b.nodes.begin() + bj + 1);
            out.push_back(m);
        }
    }
    sort(out.begin(), out.end(), [](const Move& x, const Move& y) {
        if (x.est != y.est) return x.est < y.est;
        if (x.shared != y.shared) return x.shared > y.shared;
        return x.perp < y.perp;
    });
    if ((int)out.size() > P.M) out.resize(P.M);
    return out;
}

static vector<int> bfsPath(int s, int t, const vector<int>& cnt) {   // shortest, never head-on against a used edge
    vector<int> par(V, -2); queue<int> q; par[s] = -1; q.push(s);
    while (!q.empty() && par[t] == -2) {
        int v = q.front(); q.pop();
        for (int d = 0; d < 4; d++) {
            int r = v / N + DR[d], c = v % N + DC[d];
            if (r < 0 || c < 0 || r >= N || c >= N) continue;
            int u = r * N + c;
            if (wallv[u] || par[u] != -2 || cnt[u * 4 + (d ^ 2)] > 0) continue;
            par[u] = v; q.push(u);
        }
    }
    if (par[t] == -2) return {};
    vector<int> p;
    for (int v = t; v != -1; v = par[v]) p.push_back(v);
    reverse(p.begin(), p.end());
    return p;
}

static bool applyMove(const Config& C, const Move& m, Config& out, const Params& P) {
    out = C;
    Tree& T = out[m.ta];
    vector<int> cnt = unionCount(C);
    int a0 = m.a.front(), x = m.b.front(), y = m.b.back();
    vector<int> p1 = bfsPath(a0, x, cnt), p2 = bfsPath(y, m.q, cnt);
    if (p1.empty() || p2.empty()) return false;
    vector<int> route = p1;
    for (size_t k = 1; k < m.b.size(); k++) route.push_back(m.b[k]);
    for (size_t k = 1; k < p2.size(); k++) route.push_back(p2[k]);
    vector<int> pos(V, -1), simple;                                    // loop-erase the walk
    for (int v : route) {
        if (pos[v] != -1) { while ((int)simple.size() > pos[v] + 1) { pos[simple.back()] = -1; simple.pop_back(); } }
        else { pos[v] = simple.size(); simple.push_back(v); }
    }
    for (size_t k = 0; k + 1 < simple.size(); k++) T.nxt[simple[k]] = simple[k + 1];
    vector<char> state(V, 0); state[T.root] = 2;                       // every slime must still reach the root
    for (int s : T.src) {
        vector<int> path; int v = s;
        while (true) {
            if (state[v] == 2) break;
            if (state[v] == 1) return false;                           // cycle
            state[v] = 1; path.push_back(v);
            v = T.nxt[v];
            if (v < 0) return false;
        }
        for (int q : path) state[q] = 2;
    }
    int edges = 0;
    for (int v = 0; v < V; v++) { if (state[v] != 2) T.nxt[v] = -1; else if (T.nxt[v] != -1) edges++; }
    if (P.cap > 0 && edges > P.cap) return false;
    return true;
}

struct Log { Move m; int delta; };

int main(int argc, char** argv) {
    Params P;
    if (argc > 1) P.M = atoi(argv[1]);
    if (argc > 2) P.cap = atoi(argv[2]);          // optional cap on edges per dMST (0 = off)
    if (argc > 3) P.maxGap = atoi(argv[3]);
    if (argc > 4) P.maxPerp = atoi(argv[4]);
    int W, K;
    if (scanf("%d %d %d", &N, &W, &K) != 3) return 1;
    V = N * N;
    wallv.assign(V, 0);
    for (int i = 0; i < W; i++) { int r, c; if (scanf("%d %d", &r, &c) != 2) return 1; wallv[r * N + c] = 1; }
    Config C(K);
    for (int t = 0; t < K; t++) {
        int E; if (scanf("%d", &E) != 1) return 1;
        Tree& T = C[t]; T.nxt.assign(V, -1);
        vector<int> indeg(V, 0), touch;
        for (int e = 0; e < E; e++) {
            int r1, c1, r2, c2;
            if (scanf("%d %d %d %d", &r1, &c1, &r2, &c2) != 4) return 1;
            int u = r1 * N + c1, v = r2 * N + c2;
            if (wallv[u] || wallv[v] || dirOf(u, v) < 0 || T.nxt[u] != -1) { printf("invalid input: dMST %d edge %d\n", t, e); return 2; }
            T.nxt[u] = v; indeg[v]++; touch.push_back(u); touch.push_back(v);
        }
        for (int v : touch) {
            if (T.nxt[v] == -1) T.root = v;
            if (indeg[v] == 0 && find(T.src.begin(), T.src.end(), v) == T.src.end()) T.src.push_back(v);
        }
        Config one{T}; 
        for (int s : T.src) { int v = s, st = 0; while (v != T.root && v >= 0 && st++ <= V) v = T.nxt[v];
            if (v != T.root) { printf("invalid input: dMST %d is not an in-tree\n", t); return 2; } }
    }
    D.assign(V, vector<int>(V, -1));
    for (int s = 0; s < V; s++) {
        if (wallv[s]) continue;
        queue<int> q; q.push(s); D[s][s] = 0;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int d = 0; d < 4; d++) {
                int r = v / N + DR[d], c = v % N + DC[d];
                if (r < 0 || c < 0 || r >= N || c >= N) continue;
                int u = r * N + c;
                if (!wallv[u] && D[s][u] < 0) { D[s][u] = D[s][v] + 1; q.push(u); }
            }
        }
    }
    int cur, before;
    if (!evaluate(C, cur)) { printf("invalid input: head-on collision between dMSTs\n"); return 2; }
    before = cur;
    vector<Log> log;
    for (int iter = 0; iter < 300; iter++) {
        vector<Move> moves = generate(C, P);
        int best = cur; Config bestC; Move bestM; bool found = false;
        vector<pair<Move, Config>> firsts;                             // valid first moves for the lookahead
        for (auto& m : moves) {
            Config C2; int sc;
            if (!applyMove(C, m, C2, P) || !evaluate(C2, sc)) continue;
            if (sc < best) { best = sc; bestC = C2; bestM = m; found = true; }
            else if ((int)firsts.size() < P.L) firsts.push_back({m, C2});
        }
        if (found) { log.push_back({bestM, best - cur}); C = bestC; cur = best; continue; }
        // depth-2 lookahead: tolerate a locally worse deviation if a second one makes the total better
        int best2 = cur; Config c1, c2; Move m1, m2; bool found2 = false;
        for (auto& f : firsts) {
            for (auto& m : generate(f.second, P)) {
                Config C3; int sc;
                if (!applyMove(f.second, m, C3, P) || !evaluate(C3, sc)) continue;
                if (sc < best2) { best2 = sc; c1 = f.second; c2 = C3; m1 = f.first; m2 = m; found2 = true; }
            }
        }
        if (!found2) break;
        int s1; evaluate(c1, s1);
        log.push_back({m1, s1 - cur}); log.push_back({m2, best2 - s1});
        C = c2; cur = best2;
    }
    printf("score %d %d\n", before, cur);
    printf("deviations %zu\n", log.size());
    auto rc = [&](int v) { return to_string(v / N) + "," + to_string(v % N); };
    for (auto& l : log)
        printf("DEV dMST %d path %s->%s rerouted, rides dMST %d run %s->%s (%+d)\n", l.m.ta, rc(l.m.a.front()).c_str(),
               rc(l.m.q).c_str(), l.m.tb, rc(l.m.b.front()).c_str(), rc(l.m.b.back()).c_str(), l.delta);
    for (int t = 0; t < K; t++) {
        int E = 0; for (int v = 0; v < V; v++) E += C[t].nxt[v] != -1;
        printf("TREE %d %d\n", t, E);
        for (int v = 0; v < V; v++) if (C[t].nxt[v] != -1)
            printf("%d %d %d %d\n", v / N, v % N, C[t].nxt[v] / N, C[t].nxt[v] % N);
    }
}

```