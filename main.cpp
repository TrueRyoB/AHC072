#include <bits/stdc++.h>
using namespace std;

// AHC slime-tower solver.
//
// The search state is the exact bottom-to-top content of every tower.  A
// receding-horizon beam search chooses the next move.  Besides progress toward
// nests, its evaluation rewards a reserve only through the external slimes it
// can actually receive and launch.  Height eight has no receiving capacity,
// so it receives no such credit.  When the time budget is exhausted, the solver uses
// legal one-cell shortest-path moves, including a hole-propagation routine for
// a full landing cell, so that it always has a completion route.

namespace {

constexpr int MAX_N = 20;
constexpr int MAX_V = MAX_N * MAX_N;
constexpr int MAX_K = 12;
// The official limit is tight enough that output can be truncated if an
// expensive completion routine is allowed to start after the beam deadline.
// Keep separate budgets for lookahead, short greedy improvement, and the
// cheap direct completion path.  SOLVE_SECONDS intentionally leaves time for
// formatting and flushing every operation as a complete line.
constexpr double SEARCH_SECONDS = 0.90;
constexpr double GREEDY_SECONDS = 1.18;
constexpr double SOLVE_SECONDS = 1.45;

const int DR[4] = {-1, 1, 0, 0};
const int DC[4] = {0, 0, -1, 1};
const char DCH[4] = {'U', 'D', 'L', 'R'};

int N, K, V;
int nestAt[MAX_V];
int nestPos[MAX_K];
int distToNest[MAX_K][MAX_V];
int parentToNest[MAX_K][MAX_V];
vector<int> descendingTreeOrder[MAX_K];
int landing[MAX_V][4][9];
bool floorCell[MAX_V];
long long supportedJumpBenefit[MAX_K][MAX_V][9];
int initialSlimes;

struct Tower {
    unsigned char h = 0;
    unsigned char a[8]{}; // bottom to top
};

struct State {
    array<Tower, MAX_V> t{};
    int remain = 0;
};

// The union of the K directed shortest-path trees.  flow[c][v] is the number
// of live color-c slimes whose technical shortest route still passes through
// v.  attached is demand from an otherwise isolated branch that can join v in
// the diagonal planning graph with only a small detour.
struct FlowInfo {
    array<array<short, MAX_V>, MAX_K> flow{};
    array<array<short, MAX_V>, MAX_K> local{};
    array<array<short, MAX_V>, MAX_K> attached{};
    array<short, MAX_V> total{};
    array<short, MAX_V> localTotal{};
};

struct Move {
    int from = -1;
    int keep = 0;
    int dir = 0;
    int len = 1;
    int to = -1;
    int quick = INT_MIN;
};

struct OutputMove {
    int from;
    int keep;
    int dir;
    int len;
};

vector<OutputMove> answer;
deque<uint64_t> recentStateKeys;
chrono::steady_clock::time_point started;

inline double elapsedSeconds() {
    return chrono::duration<double>(chrono::steady_clock::now() - started).count();
}

inline bool inBoard(int r, int c) {
    return 0 <= r && r < N && 0 <= c && c < N;
}

inline int cellOf(int r, int c) {
    return r * N + c;
}

uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

uint64_t stateHash(const State& s) {
    uint64_t h = splitmix64(static_cast<uint64_t>(s.remain));
    for (int p = 0; p < V; ++p) {
        const Tower& x = s.t[p];
        for (int i = 0; i < x.h; ++i) {
            h ^= splitmix64((static_cast<uint64_t>(p + 1) << 16)
                ^ (static_cast<uint64_t>(i + 1) << 8) ^ x.a[i]);
        }
    }
    return h;
}

bool recentlySeen(uint64_t key) {
    return find(recentStateKeys.begin(), recentStateKeys.end(), key) != recentStateKeys.end();
}

void rememberState(const State& s) {
    recentStateKeys.push_back(stateHash(s));
    constexpr int TABU_STATES = 32;
    if (static_cast<int>(recentStateKeys.size()) > TABU_STATES) recentStateKeys.pop_front();
}

void normalize(State& s, int p) {
    const int color = nestAt[p];
    if (color < 0) return;
    Tower& x = s.t[p];
    while (x.h > 0 && x.a[x.h - 1] == color) {
        --x.h;
        --s.remain;
    }
}

// The move is assumed legal.  Keeping this function as the sole state
// transition is important: beam nodes and printed moves use exactly the same
// simulation, including the two automatic homing steps.
void applyMove(State& s, const Move& mv) {
    Tower& src = s.t[mv.from];
    Tower& dst = s.t[mv.to];
    const int oldH = src.h;
    const int moved = oldH - mv.keep;
    unsigned char tmp[8];
    for (int i = 0; i < moved; ++i) tmp[i] = src.a[oldH - 1 - i];
    src.h = static_cast<unsigned char>(mv.keep);
    for (int i = 0; i < moved; ++i) dst.a[dst.h++] = tmp[i];
    normalize(s, mv.from);
    normalize(s, mv.to);
}

// Linear distance alone lets a remote singleton remain ignored while the beam
// improves a convenient local cluster.  This term is deliberately gentle for
// a short local stretch, then becomes convex and eventually doubles by distance
// bands.  The exponential part is capped: a useful detour can still survive
// if it creates a genuinely valuable scaffold.
long long routeCost(int d) {
    const int excess = max(0, d - 6);
    const int band = min(12, (excess + 4) / 5);
    const long long curved = 2LL * excess * excess;
    const long long exponential = 8LL * ((1LL << band) - 1LL);
    return 45LL * d + curved + exponential;
}

FlowInfo buildCombinedFlow(const State& s) {
    FlowInfo info;
    for (int p = 0; p < V; ++p) {
        const Tower& x = s.t[p];
        for (int i = 0; i < x.h; ++i) {
            const int color = x.a[i];
            ++info.local[color][p];
            ++info.flow[color][p];
            ++info.localTotal[p];
        }
    }

    // Accumulate every live token upward in its color's directed shortest
    // path tree.  The resulting union is our inexpensive dMST-like technical
    // lower-bound graph; it is rooted at every nest simultaneously.
    for (int color = 0; color < K; ++color) {
        for (int p : descendingTreeOrder[color]) {
            const int parent = parentToNest[color][p];
            if (parent >= 0) info.flow[color][parent] += info.flow[color][p];
        }
    }
    for (int p = 0; p < V; ++p) {
        for (int color = 0; color < K; ++color) info.total[p] += info.flow[color][p];
    }

    // A joint is a point with multiple units of technical flow, or flow from
    // several colors.  We extend those joints in an 8-neighbor planning graph
    // so an isolated branch can attach diagonally when that does not create a
    // material detour.  This is only an evaluation graph; every actual move
    // remains a legal orthogonal jump.
    vector<int> joints;
    for (int p = 0; p < V; ++p) {
        if (!floorCell[p] || info.total[p] < 2) continue;
        int colors = 0;
        for (int c = 0; c < K; ++c) colors += info.flow[c][p] > 0;
        if (colors >= 2 || info.total[p] - info.localTotal[p] >= 2) joints.push_back(p);
    }
    if (joints.empty()) return info;

    static const int D8R[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const int D8C[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    array<int, MAX_V> nearestJoint, diagonalDistance;
    nearestJoint.fill(-1);
    diagonalDistance.fill(-1);
    queue<int> que;
    for (int p : joints) {
        nearestJoint[p] = p;
        diagonalDistance[p] = 0;
        que.push(p);
    }
    while (!que.empty()) {
        const int v = que.front();
        que.pop();
        const int r = v / N, c = v % N;
        for (int d = 0; d < 8; ++d) {
            const int nr = r + D8R[d], nc = c + D8C[d];
            if (!inBoard(nr, nc)) continue;
            const int u = cellOf(nr, nc);
            if (!floorCell[u] || diagonalDistance[u] >= 0) continue;
            nearestJoint[u] = nearestJoint[v];
            diagonalDistance[u] = diagonalDistance[v] + 1;
            que.push(u);
        }
    }

    for (int p = 0; p < V; ++p) {
        if (s.t[p].h == 0 || info.total[p] != info.localTotal[p]) continue;
        const int joint = nearestJoint[p];
        if (joint < 0 || joint == p || diagonalDistance[p] > 3) continue;
        for (int color = 0; color < K; ++color) {
            const int count = info.local[color][p];
            if (count == 0) continue;
            const int detour = diagonalDistance[p] + distToNest[color][joint] - distToNest[color][p];
            if (detour <= 2) info.attached[color][joint] += count;
        }
    }
    return info;
}

// A reserve has no intrinsic score.  Its score is the sigma of the jump
// benefit it offers to external, unresolved flow in the combined forest.
long long externalSupportPotential(const State& s, const FlowInfo& info) {
    long long total = 0;
    for (int p = 0; p < V; ++p) {
        const int support = s.t[p].h;
        if (support == 0 || support >= 8 || nestAt[p] >= 0) continue;
        for (int color = 0; color < K; ++color) {
            const int external = info.flow[color][p] - info.local[color][p] + info.attached[color][p];
            if (external <= 0) continue;
            total += 1LL * external * supportedJumpBenefit[color][p][support];
        }
    }
    return total;
}

// Moving a base is expensive in proportion to the concrete multicolor demand
// it stops supporting.  This is used to order candidates, not as a hard
// legality condition: a direct completion move remains possible when needed.
long long releaseDebt(const FlowInfo& info, int p, int before, int after) {
    if (before == after) return 0;
    long long debt = 0;
    for (int color = 0; color < K; ++color) {
        const int external = info.flow[color][p] - info.local[color][p] + info.attached[color][p];
        if (external <= 0) continue;
        const long long oldBenefit = (0 < before && before < 8)
            ? supportedJumpBenefit[color][p][before] : 0;
        const long long newBenefit = (0 < after && after < 8)
            ? supportedJumpBenefit[color][p][after] : 0;
        if (oldBenefit > newBenefit) {
            debt += external * (oldBenefit - newBenefit + 15000LL);
        }
    }
    return debt;
}

// A contiguous top run of one color can move as one group even with no
// external reserve.  Each extra slime in that run saves roughly one operation
// per remaining path step, so this is the cumulative-distance advantage that
// makes same-color coalescence preferable to repeatedly using a stack only as
// a temporary scaffold.
long long sameColorCoalescenceValue(const State& s) {
    long long value = 0;
    for (int p = 0; p < V; ++p) {
        const Tower& x = s.t[p];
        if (x.h < 2) continue;
        const int color = x.a[x.h - 1];
        int run = 1;
        while (run < x.h && x.a[x.h - 1 - run] == color) ++run;
        if (run < 2) continue;
        const int d = distToNest[color][p];
        value += 6LL * (run - 1) * routeCost(d);
    }
    return value;
}

// Find the nearest *other* occupied tower for every tower with one
// multi-source BFS.  A singleton cannot use a reserve, move a useful group,
// or be carried until it has caught up with another tower.  Towers with two or
// more slimes already have their own local cooperation, so the strong part of
// this penalty applies only to singleton components.
long long topologicalPenalty(const State& s) {
    vector<int> sources;
    sources.reserve(V);
    for (int p = 0; p < V; ++p) if (s.t[p].h > 0) sources.push_back(p);
    if (sources.size() <= 1) return 0;

    array<int, MAX_V> owner, depth;
    owner.fill(-1);
    depth.fill(-1);
    vector<int> nearest(sources.size(), INT_MAX);
    queue<int> que;
    for (int i = 0; i < static_cast<int>(sources.size()); ++i) {
        owner[sources[i]] = i;
        depth[sources[i]] = 0;
        que.push(sources[i]);
    }
    while (!que.empty()) {
        const int v = que.front();
        que.pop();
        const int r = v / N, c = v % N;
        for (int dir = 0; dir < 4; ++dir) {
            const int nr = r + DR[dir], nc = c + DC[dir];
            if (!inBoard(nr, nc)) continue;
            const int u = cellOf(nr, nc);
            if (!floorCell[u]) continue;
            if (owner[u] < 0) {
                owner[u] = owner[v];
                depth[u] = depth[v] + 1;
                que.push(u);
            } else if (owner[u] != owner[v]) {
                const int d = depth[u] + depth[v] + 1;
                nearest[owner[u]] = min(nearest[owner[u]], d);
                nearest[owner[v]] = min(nearest[owner[v]], d);
            }
        }
    }

    long long penalty = 0;
    for (int i = 0; i < static_cast<int>(sources.size()); ++i) {
        const int p = sources[i];
        const Tower& x = s.t[p];
        const int routeD = distToNest[x.a[x.h - 1]][p];
        if (x.h == 1) {
            // Cap the gap term.  Otherwise two final, unavoidable singleton
            // slimes in opposite sides of a long maze would dominate a
            // sensible completion route to their own nests.
            const int gap = min(16, max(0, nearest[i] - 2));
            penalty += 70LL * gap * gap;
            penalty += 10LL * gap * max(0, routeD - 6);
        }
    }
    return penalty;
}

long long stateValue(const State& s) {
    const FlowInfo flow = buildCombinedFlow(s);
    long long routePenalty = 0;
    long long blockedNest = 0;
    for (int p = 0; p < V; ++p) {
        const Tower& x = s.t[p];
        if (x.h == 0) continue;
        if (nestAt[p] >= 0) blockedNest += 12LL * x.h;
        long long cellCost = 0;
        for (int i = 0; i < x.h; ++i) cellCost += routeCost(distToNest[x.a[i]][p]);

        const long long heightWeight = 8 + x.h;
        routePenalty += cellCost * heightWeight / 9LL;
    }
    const long long completed = initialSlimes - s.remain;
    return completed * 50000LL - routePenalty + externalSupportPotential(s, flow)
        + sameColorCoalescenceValue(s) - blockedNest - topologicalPenalty(s);
}

int predictedHomes(const State& s, int from, int keep, int to) {
    const Tower& a = s.t[from];
    int homes = 0;
    const int dstColor = nestAt[to];
    if (dstColor >= 0) {
        // After reversal, a[keep] is the topmost arriving slime.
        for (int i = keep; i < a.h && a.a[i] == dstColor; ++i) ++homes;
    }
    const int srcColor = nestAt[from];
    if (srcColor >= 0) {
        for (int i = keep - 1; i >= 0 && a.a[i] == srcColor; --i) ++homes;
    }
    return homes;
}

vector<Move> generateMoves(const State& s, int limit, bool completionMode,
                           int forbidFrom = -1, int forbidTo = -1) {
    const FlowInfo flow = buildCombinedFlow(s);
    vector<Move> candidates;
    candidates.reserve(5000);

    for (int p = 0; p < V; ++p) {
        const Tower& a = s.t[p];
        const int h = a.h;
        if (h == 0) continue;

        for (int keep = 0; keep < h; ++keep) {
            const int moved = h - keep;
            int oldDistance = 0;
            for (int i = keep; i < h; ++i) oldDistance += distToNest[a.a[i]][p];

            for (int dir = 0; dir < 4; ++dir) {
                for (int len = 1; len <= keep + 1; ++len) {
                    const int q = landing[p][dir][len];
                    if (q < 0) break;
                    if (p == forbidFrom && q == forbidTo) continue;
                    const Tower& b = s.t[q];
                    if (b.h + moved > 8) continue;

                    int newDistance = 0;
                    long long nonlinearGain = 0;
                    for (int i = keep; i < h; ++i) {
                        const int beforeDistance = distToNest[a.a[i]][p];
                        const int afterDistance = distToNest[a.a[i]][q];
                        newDistance += afterDistance;
                        nonlinearGain += routeCost(beforeDistance) - routeCost(afterDistance);
                    }
                    const int gain = oldDistance - newDistance;
                    const int homes = predictedHomes(s, p, keep, q);

                    int score;
                    if (completionMode) {
                        // Once the beam budget is gone, keep the monotone
                        // distance objective dominant.  Larger groups and
                        // longer valid jumps break otherwise common ties.
                        score = homes * 1000000 + gain * 1000 + moved * len;
                        if (gain <= 0 && homes == 0) score -= 10000000;
                    } else {
                        const int boundedGain = static_cast<int>(max(-2000000LL, min(2000000LL, nonlinearGain)));
                        score = homes * 100000 + boundedGain
                            + 3 * moved * len;
                        // Reaching another live tower is the only local
                        // evidence that a newly formed stack can help someone
                        // else.  Height at an empty cell earns no such bonus.
                        if (b.h > 0) score += 80 * moved;
                        // A singleton can only travel one cell by itself.  A
                        // move that catches it up to an existing tower is a
                        // topological improvement even when its own nest
                        // distance temporarily rises.
                        if (h == 1 && b.h > 0) score += 420 + 40 * b.h;
                        if (nestAt[q] >= 0 && homes == 0) score -= 30;
                        // Fast candidate-level approximation of the exact
                        // coalescence value above.  It only fires when both
                        // sides form an exposed contiguous run of one color.
                        const int mergeColor = a.a[keep];
                        bool monochrome = true;
                        for (int i = keep + 1; i < h; ++i) monochrome &= a.a[i] == mergeColor;
                        if (monochrome && b.h > 0 && b.a[b.h - 1] == mergeColor) {
                            int destinationRun = 1;
                            while (destinationRun < b.h && b.a[b.h - 1 - destinationRun] == mergeColor) {
                                ++destinationRun;
                            }
                            const long long mergeBonus = 6LL * min(moved, destinationRun)
                                * routeCost(distToNest[mergeColor][q]);
                            score += static_cast<int>(min(2000000LL, mergeBonus));
                        }
                        const long long debt = releaseDebt(flow, p, h, keep);
                        score -= static_cast<int>(min(2000000LL, debt));
                    }
                    candidates.push_back({p, keep, dir, len, q, score});
                }
            }
        }
    }

    auto better = [](const Move& a, const Move& b) {
        if (a.quick != b.quick) return a.quick > b.quick;
        if (a.len != b.len) return a.len > b.len;
        if (a.keep != b.keep) return a.keep > b.keep;
        return a.from < b.from;
    };
    if (static_cast<int>(candidates.size()) > limit) {
        nth_element(candidates.begin(), candidates.begin() + limit, candidates.end(), better);
        candidates.resize(limit);
    }
    sort(candidates.begin(), candidates.end(), better);
    return candidates;
}

struct Node {
    State state;
    long long value = LLONG_MIN;
    Move first;
    bool hasFirst = false;
    int lastFrom = -1;
    int lastTo = -1;
    uint64_t key = 0;
};

bool betterNode(const Node& a, const Node& b) {
    if (a.value != b.value) return a.value > b.value;
    if (a.state.remain != b.state.remain) return a.state.remain < b.state.remain;
    return a.first.quick > b.first.quick;
}

bool chooseBeamMove(const State& root, Move& chosen) {
    // The old four-ply horizon could see a base being used but not the later
    // merge or release of that base.  Eight plies are enough to compare those
    // short dependency chains while the smaller frontier keeps work similar.
    constexpr int WIDTH = 10;
    constexpr int DEPTH = 8;
    constexpr int BRANCH = 16;

    vector<Node> beam;
    Node initial;
    initial.state = root;
    initial.value = stateValue(root);
    initial.key = stateHash(root);
    beam.push_back(std::move(initial));
    const uint64_t rootKey = beam.front().key;
    bool found = false;

    for (int depth = 0; depth < DEPTH; ++depth) {
        if (elapsedSeconds() >= SEARCH_SECONDS) break;
        vector<Node> next;
        next.reserve(WIDTH * BRANCH);
        for (const Node& node : beam) {
            vector<Move> moves = generateMoves(node.state, BRANCH, false, node.lastTo, node.lastFrom);
            for (const Move& mv : moves) {
                State child = node.state;
                applyMove(child, mv);
                Node x;
                x.state = child;
                x.value = stateValue(child);
                x.first = node.hasFirst ? node.first : mv;
                x.hasFirst = true;
                x.lastFrom = mv.from;
                x.lastTo = mv.to;
                x.key = stateHash(child);
                if (x.key == rootKey || recentlySeen(x.key)) x.value -= 1000000000000LL;
                next.push_back(std::move(x));
            }
        }
        if (next.empty()) break;
        sort(next.begin(), next.end(), betterNode);
        unordered_set<uint64_t> used;
        vector<Node> unique;
        unique.reserve(WIDTH);
        for (Node& node : next) {
            if (!used.insert(node.key).second) continue;
            unique.push_back(std::move(node));
            if (static_cast<int>(unique.size()) == WIDTH) break;
        }
        if (unique.empty()) break;
        beam.swap(unique);
        found = true;
    }
    // All nodes of the retained frontier represent the same number of future
    // operations.  Selecting this deepest completed frontier is what lets a
    // temporary reserve win over an attractive one-move greedy jump.
    if (found) chosen = beam.front().first;
    return found;
}

void emitAndApply(State& s, const Move& mv) {
    answer.push_back({mv.from, mv.keep, mv.dir, mv.len});
    applyMove(s, mv);
    rememberState(s);
}

// If p is full, propagate an available slot backwards along a shortest grid
// path from p to a cell of height < 8.  Every shift is a legal top-one jump of
// length one.  This is a completion device, not part of the beam heuristic.
bool makeRoom(State& s, int p, int avoid = -1) {
    if (s.t[p].h < 8) return true;
    array<int, MAX_V> parent;
    parent.fill(-2);
    queue<int> que;
    parent[p] = -1;
    que.push(p);
    int hole = -1;
    while (!que.empty()) {
        const int v = que.front();
        que.pop();
        if (v != avoid && s.t[v].h < 8) {
            hole = v;
            break;
        }
        const int r = v / N, c = v % N;
        for (int d = 0; d < 4; ++d) {
            const int nr = r + DR[d], nc = c + DC[d];
            if (!inBoard(nr, nc)) continue;
            const int u = cellOf(nr, nc);
            if (!floorCell[u] || u == avoid || parent[u] != -2) continue;
            parent[u] = v;
            que.push(u);
        }
    }
    if (hole < 0) return false; // impossible here because total capacity exceeds live slimes

    vector<int> path;
    for (int v = hole; v != -1; v = parent[v]) path.push_back(v);
    reverse(path.begin(), path.end()); // p ... hole
    for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
        const int u = path[i], v = path[i + 1];
        int dir = -1;
        for (int d = 0; d < 4; ++d) {
            if (landing[u][d][1] == v) {
                dir = d;
                break;
            }
        }
        if (dir < 0 || s.t[u].h == 0 || s.t[v].h >= 8) return false;
        emitAndApply(s, {u, static_cast<int>(s.t[u].h) - 1, dir, 1, v, 0});
        if (answer.size() >= 100000) return false;
    }
    return s.t[p].h < 8;
}

// A deterministic, terminating fallback.  It first takes the best move that
// lowers aggregate nest distance, then uses a top slime's shortest path if
// needed.  The latter is enough to finish because makeRoom can always create
// capacity in the next cell.
bool directOneStep(State& s) {
    int bestP = -1;
    int bestD = -1;
    for (int p = 0; p < V; ++p) {
        const Tower& x = s.t[p];
        if (x.h == 0) continue;
        const int color = x.a[x.h - 1];
        if (distToNest[color][p] > bestD) {
            bestD = distToNest[color][p];
            bestP = p;
        }
    }
    if (bestP < 0) return false;

    const Tower& source = s.t[bestP];
    const int color = source.a[source.h - 1];
    const int wanted = distToNest[color][bestP] - 1;
    int dir = -1, to = -1;
    for (int d = 0; d < 4; ++d) {
        const int q = landing[bestP][d][1];
        if (q >= 0 && distToNest[color][q] == wanted) {
            dir = d;
            to = q;
            break;
        }
    }
    if (dir < 0) return false;
    if (s.t[to].h >= 8) {
        // A propagation path can include bestP, in which case its original top
        // slime may already have made the desired step.  Replan on the next
        // outer iteration instead of relying on stale stack information.
        if (makeRoom(s, to, bestP)) return true;
        return makeRoom(s, to);
    }
    emitAndApply(s, {bestP, static_cast<int>(source.h) - 1, dir, 1, to, 0});
    return true;
}

// Use aggregate-progress moves only while there is an explicit budget for
// searching them.  directOneStep is O(N^2) and is used thereafter, so the
// time spent after the beam phase is bounded by the number of emitted moves.
bool completeOneStep(State& s) {
    vector<Move> progress = generateMoves(s, 1, true);
    if (!progress.empty() && progress.front().quick > 0) {
        emitAndApply(s, progress.front());
        return true;
    }
    return directOneStep(s);
}

void computeDistances() {
    for (int color = 0; color < K; ++color) {
        fill(distToNest[color], distToNest[color] + V, -1);
        fill(parentToNest[color], parentToNest[color] + V, -1);
        queue<int> que;
        distToNest[color][nestPos[color]] = 0;
        que.push(nestPos[color]);
        while (!que.empty()) {
            const int v = que.front();
            que.pop();
            const int r = v / N, c = v % N;
            for (int d = 0; d < 4; ++d) {
                const int nr = r + DR[d], nc = c + DC[d];
                if (!inBoard(nr, nc)) continue;
                const int u = cellOf(nr, nc);
                if (!floorCell[u] || distToNest[color][u] != -1) continue;
                distToNest[color][u] = distToNest[color][v] + 1;
                parentToNest[color][u] = v;
                que.push(u);
            }
        }
        descendingTreeOrder[color].clear();
        for (int p = 0; p < V; ++p) if (floorCell[p]) descendingTreeOrder[color].push_back(p);
        sort(descendingTreeOrder[color].begin(), descendingTreeOrder[color].end(), [color](int a, int b) {
            return distToNest[color][a] > distToNest[color][b];
        });
    }
}

void computeLanding() {
    for (int p = 0; p < V; ++p) {
        for (int d = 0; d < 4; ++d) {
            for (int len = 0; len <= 8; ++len) landing[p][d][len] = -1;
            if (!floorCell[p]) continue;
            const int r = p / N, c = p % N;
            bool clear = true;
            for (int len = 1; len <= 8; ++len) {
                const int nr = r + DR[d] * len, nc = c + DC[d] * len;
                if (!inBoard(nr, nc) || !floorCell[cellOf(nr, nc)]) clear = false;
                if (clear) landing[p][d][len] = cellOf(nr, nc);
            }
        }
    }
}

void computeSupportedJumpBenefits() {
    for (int color = 0; color < K; ++color) {
        for (int p = 0; p < V; ++p) {
            for (int support = 0; support <= 8; ++support) supportedJumpBenefit[color][p][support] = 0;
            if (!floorCell[p]) continue;
            for (int support = 1; support < 8; ++support) {
                long long best = 0;
                for (int dir = 0; dir < 4; ++dir) {
                    for (int len = 1; len <= support + 1; ++len) {
                        const int q = landing[p][dir][len];
                        if (q < 0) break;
                        best = max(best, routeCost(distToNest[color][p]) - routeCost(distToNest[color][q]));
                    }
                }
                supportedJumpBenefit[color][p][support] = best;
            }
        }
    }
}

} // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N >> K)) return 0;
    V = N * N;
    fill(nestAt, nestAt + MAX_V, -1);
    fill(nestPos, nestPos + MAX_K, -1);
    fill(floorCell, floorCell + MAX_V, false);

    vector<string> board(N);
    State current;
    for (int r = 0; r < N; ++r) {
        cin >> board[r];
        for (int c = 0; c < N; ++c) {
            const char ch = board[r][c];
            const int p = cellOf(r, c);
            if (ch == '#') continue;
            floorCell[p] = true;
            if ('A' <= ch && ch <= 'L') {
                const int color = ch - 'A';
                nestAt[p] = color;
                nestPos[color] = p;
            } else if ('a' <= ch && ch <= 'l') {
                const int color = ch - 'a';
                current.t[p].a[0] = static_cast<unsigned char>(color);
                current.t[p].h = 1;
                ++current.remain;
            }
        }
    }
    initialSlimes = current.remain;
    computeDistances();
    computeLanding();
    computeSupportedJumpBenefits();
    started = chrono::steady_clock::now();
    rememberState(current);

    while (current.remain > 0 && answer.size() < 100000) {
        if (elapsedSeconds() >= SOLVE_SECONDS) break;
        // With exactly one live slime there is no external user for any
        // scaffold.  A shortest-path step is therefore strictly dominant and
        // prevents a receding beam from spending moves on a fake setup.
        if (current.remain == 1) {
            if (!directOneStep(current)) break;
            continue;
        }
        Move mv;
        if (elapsedSeconds() < SEARCH_SECONDS && chooseBeamMove(current, mv)) {
            emitAndApply(current, mv);
        } else if (elapsedSeconds() < GREEDY_SECONDS && completeOneStep(current)) {
            continue;
        } else if (!directOneStep(current)) {
            break;
        }
    }

    // Construct the complete stream before its first write.  This removes the
    // many small iostream insertions which made a timeout more likely to leave
    // a syntactically incomplete final line.
    string output;
    output.reserve(answer.size() * 14);
    for (const OutputMove& mv : answer) {
        output += to_string(mv.from / N);
        output.push_back(' ');
        output += to_string(mv.from % N);
        output.push_back(' ');
        output += to_string(mv.keep);
        output.push_back(' ');
        output.push_back(DCH[mv.dir]);
        output.push_back(' ');
        output += to_string(mv.len);
        output.push_back('\n');
    }
    cout << output;
    return 0;
}
