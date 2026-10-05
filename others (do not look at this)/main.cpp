#include <bits/stdc++.h>
using namespace std;

constexpr int MAX_ACTIONS = 100000;
constexpr int MAX_HEIGHT = 8;
constexpr int DR[4] = {0, 1, 0, -1};
constexpr int DC[4] = {1, 0, -1, 0};
constexpr char DIR[4] = {'R', 'D', 'L', 'U'};

struct Point {
    int r, c;
    bool operator==(const Point& other) const { return r == other.r && c == other.c; }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

struct Move {
    int r, c, keep, dir, len;
};

struct SlimeRef {
    Point p;
    int color;
};

struct Board {
    int n;
    vector<string> terrain;
    vector<vector<int>> nest;
    vector<vector<vector<int>>> stack_at;

    bool inside(Point p) const {
        return 0 <= p.r && p.r < n && 0 <= p.c && p.c < n;
    }

    bool floor(Point p) const {
        return inside(p) && terrain[p.r][p.c] != '#';
    }

    int height(Point p) const {
        return static_cast<int>(stack_at[p.r][p.c].size());
    }

    int top_color(Point p) const {
        const auto& s = stack_at[p.r][p.c];
        return s.empty() ? -1 : s.back();
    }

    int top_run(Point p, int color) const {
        const auto& s = stack_at[p.r][p.c];
        int result = 0;
        while (result < static_cast<int>(s.size()) && s[s.size() - 1 - result] == color) ++result;
        return result;
    }

    void homecoming(Point p) {
        const int wanted = nest[p.r][p.c];
        if (wanted < 0) return;
        auto& s = stack_at[p.r][p.c];
        while (!s.empty() && s.back() == wanted) s.pop_back();
    }

    bool apply(const Move& move) {
        Point from{move.r, move.c};
        if (!floor(from) || move.dir < 0 || move.dir >= 4) return false;
        auto& source = stack_at[from.r][from.c];
        const int h = static_cast<int>(source.size());
        if (move.keep < 0 || move.keep >= h || move.len < 1 || move.len > move.keep + 1) return false;

        Point to = from;
        for (int step = 0; step < move.len; ++step) {
            to.r += DR[move.dir];
            to.c += DC[move.dir];
            if (!floor(to)) return false;
        }

        const int moving = h - move.keep;
        if (height(to) + moving > MAX_HEIGHT) return false;
        vector<int> block(source.begin() + move.keep, source.end());
        source.resize(move.keep);
        homecoming(from);
        reverse(block.begin(), block.end());
        auto& destination = stack_at[to.r][to.c];
        destination.insert(destination.end(), block.begin(), block.end());
        homecoming(to);
        return true;
    }

    bool empty() const {
        for (const auto& row : stack_at) for (const auto& s : row) if (!s.empty()) return false;
        return true;
    }
};

vector<vector<int>> block_distances(const Board& board, Point start, int moving, Point blocked = {-1, -1}) {
    vector<vector<int>> dist(board.n, vector<int>(board.n, -1));
    if (!board.floor(start) || start == blocked) return dist;
    queue<Point> q;
    dist[start.r][start.c] = 0;
    q.push(start);
    while (!q.empty()) {
        const Point cur = q.front();
        q.pop();
        for (int d = 0; d < 4; ++d) {
            Point next{cur.r + DR[d], cur.c + DC[d]};
            if (!board.floor(next) || next == blocked || dist[next.r][next.c] != -1) continue;
            if (next != start && board.height(next) + moving > MAX_HEIGHT) continue;
            dist[next.r][next.c] = dist[cur.r][cur.c] + 1;
            q.push(next);
        }
    }
    return dist;
}

vector<Point> block_path(const Board& board, Point start, Point goal, int moving, Point blocked = {-1, -1}) {
    vector<vector<Point>> parent(board.n, vector<Point>(board.n, {-1, -1}));
    if (!board.floor(start) || !board.floor(goal) || start == blocked) return {};
    queue<Point> q;
    parent[start.r][start.c] = start;
    q.push(start);
    while (!q.empty()) {
        const Point cur = q.front();
        q.pop();
        if (cur == goal) break;
        for (int d = 0; d < 4; ++d) {
            Point next{cur.r + DR[d], cur.c + DC[d]};
            if (!board.floor(next) || next == blocked || parent[next.r][next.c].r != -1) continue;
            if (next != start && board.height(next) + moving > MAX_HEIGHT) continue;
            parent[next.r][next.c] = cur;
            q.push(next);
        }
    }
    if (parent[goal.r][goal.c].r == -1) return {};
    vector<Point> path;
    for (Point cur = goal;; cur = parent[cur.r][cur.c]) {
        path.push_back(cur);
        if (cur == start) break;
    }
    reverse(path.begin(), path.end());
    return path;
}

struct StationSpec {
    Point p;
    vector<SlimeRef> base;
    vector<SlimeRef> users;
    int proxy_score = 0;
};

struct Planner {
    Board board;
    vector<Move> moves;
    bool valid = true;

    bool emit(const Move& move) {
        if (!valid || static_cast<int>(moves.size()) >= MAX_ACTIONS || !board.apply(move)) {
            valid = false;
            return false;
        }
        moves.push_back(move);
        return true;
    }

    // Moves an ordered monochromatic top block.  With merge_same enabled, every
    // same-color landing grows the block before its next departure.
    bool move_group(Point start, Point goal, int color, bool merge_same,
                    Point blocked = {-1, -1}, int fixed_count = -1) {
        if (board.top_color(start) != color) return false;
        int count = fixed_count >= 0 ? fixed_count : board.top_run(start, color);
        if (count <= 0 || board.top_run(start, color) < count) return false;
        Point cur = start;
        for (int guard = 0; cur != goal && guard < 2000; ++guard) {
            const vector<Point> path = block_path(board, cur, goal, count, blocked);
            if (path.size() < 2) return false;
            int dir = -1;
            const int step_r = path[1].r - cur.r;
            const int step_c = path[1].c - cur.c;
            for (int d = 0; d < 4; ++d) if (DR[d] == step_r && DC[d] == step_c) dir = d;
            if (dir < 0) return false;

            int straight = 0;
            while (straight + 1 < static_cast<int>(path.size())) {
                const Point a = path[straight];
                const Point b = path[straight + 1];
                if (b.r - a.r != DR[dir] || b.c - a.c != DC[dir]) break;
                ++straight;
            }
            const int keep = board.height(cur) - count;
            const int len = min(straight, keep + 1);
            if (len < 1) return false;
            const Point landing = path[len];
            const int joined = merge_same ? board.top_run(landing, color) : 0;
            if (!emit({cur.r, cur.c, keep, dir, len})) return false;
            cur = landing;
            if (cur == goal) return true;
            if (merge_same) count += joined;
            if (board.top_run(cur, color) < count) return false;
        }
        return cur == goal;
    }

    // Clears the current board.  It first chooses a profitable same-color merge;
    // otherwise it sends one complete monochromatic top run to its own nest.
    bool finish(const vector<Point>& nests) {
        for (int rounds = 0; !board.empty() && rounds < 2000; ++rounds) {
            struct Choice { Point from, to; int color, gain, distance; bool merge; };
            optional<Choice> best_merge, best_direct;

            for (int r = 0; r < board.n; ++r) for (int c = 0; c < board.n; ++c) {
                Point from{r, c};
                const int color = board.top_color(from);
                if (color < 0) continue;
                const int count = board.top_run(from, color);
                const auto dist = block_distances(board, from, count);
                const int to_nest = dist[nests[color].r][nests[color].c];
                if (to_nest >= 0 && (!best_direct || to_nest > best_direct->distance)) {
                    best_direct = Choice{from, nests[color], color, 0, to_nest, false};
                }
                if (to_nest < 0) continue;

                for (int rr = 0; rr < board.n; ++rr) for (int cc = 0; cc < board.n; ++cc) {
                    Point other{rr, cc};
                    if (other == from || board.top_color(other) != color) continue;
                    const int to_other = dist[rr][cc];
                    const int gain = to_nest - to_other;
                    if (to_other >= 0 && gain > 0 && (!best_merge || gain > best_merge->gain)) {
                        best_merge = Choice{from, other, color, gain, to_other, true};
                    }
                }
            }

            if (best_merge) {
                if (move_group(best_merge->from, best_merge->to, best_merge->color, true)) continue;
                valid = false;
                return false;
            }
            if (best_direct) {
                if (move_group(best_direct->from, best_direct->to, best_direct->color, true)) continue;
                valid = false;
                return false;
            }

            // A full run can occasionally have no height-safe route.  Move its
            // top slime so a later step can re-form the run instead of stalling.
            bool split_progress = false;
            for (int r = 0; r < board.n && !split_progress; ++r) for (int c = 0; c < board.n && !split_progress; ++c) {
                Point from{r, c};
                const int color = board.top_color(from);
                if (color < 0) continue;
                if (move_group(from, nests[color], color, true, {-1, -1}, 1)) split_progress = true;
            }
            if (!split_progress) return false;
        }
        return valid && board.empty();
    }
};

int first_straight_run(const Board& board, Point start, Point goal) {
    const vector<Point> path = block_path(board, start, goal, 1);
    if (path.size() < 2) return 0;
    const int dr = path[1].r - path[0].r;
    const int dc = path[1].c - path[0].c;
    int run = 0;
    while (run + 1 < static_cast<int>(path.size())) {
        if (path[run + 1].r - path[run].r != dr || path[run + 1].c - path[run].c != dc) break;
        ++run;
    }
    return run;
}

vector<StationSpec> station_specs(const Board& initial, const vector<SlimeRef>& slime,
                                  const vector<Point>& nests, int colors) {
    vector<int> direct(slime.size(), -1);
    for (int color = 0; color < colors; ++color) {
        const auto dist = block_distances(initial, nests[color], 1);
        for (int id = 0; id < static_cast<int>(slime.size()); ++id) {
            if (slime[id].color == color) direct[id] = dist[slime[id].p.r][slime[id].p.c];
        }
    }

    vector<StationSpec> result;
    for (int seed = 0; seed < static_cast<int>(slime.size()); ++seed) {
        const Point station = slime[seed].p;
        vector<vector<vector<int>>> approach(colors);
        vector<int> delivery(colors, -1), run(colors, 0);
        for (int color = 0; color < colors; ++color) {
            approach[color] = block_distances(initial, station, 1, nests[color]);
            const auto from_station = block_distances(initial, station, 1);
            delivery[color] = from_station[nests[color].r][nests[color].c];
            run[color] = first_straight_run(initial, station, nests[color]);
        }

        vector<bool> chosen(slime.size(), false), base_color(colors, false);
        vector<int> base_id{seed};
        chosen[seed] = true;
        base_color[slime[seed].color] = true;
        for (int need = 1; need < 4; ++need) {
            int pick = -1;
            for (int id = 0; id < static_cast<int>(slime.size()); ++id) {
                if (chosen[id]) continue;
                const int d = approach[slime[id].color][slime[id].p.r][slime[id].p.c];
                if (d < 0 || delivery[slime[id].color] < 0) continue;
                if (pick < 0) { pick = id; continue; }
                const bool new_color = !base_color[slime[id].color];
                const bool old_new_color = !base_color[slime[pick].color];
                const int old_d = approach[slime[pick].color][slime[pick].p.r][slime[pick].p.c];
                if (new_color != old_new_color ? new_color : d < old_d) pick = id;
            }
            if (pick < 0) break;
            chosen[pick] = true;
            base_color[slime[pick].color] = true;
            base_id.push_back(pick);
        }
        if (static_cast<int>(base_id.size()) != 4) continue;

        int base_cost = 0;
        for (int i = 1; i < 4; ++i) {
            const int id = base_id[i];
            const int via = approach[slime[id].color][slime[id].p.r][slime[id].p.c] + delivery[slime[id].color];
            base_cost += max(0, via - direct[id]);
        }

        // The last donor is the protected top of the base.  Excluding its color
        // from users prevents a same-color user from being deliberately kept apart.
        const int protected_color = slime[base_id.back()].color;
        vector<pair<int, int>> gain;
        for (int id = 0; id < static_cast<int>(slime.size()); ++id) {
            if (chosen[id] || slime[id].color == protected_color) continue;
            const int color = slime[id].color;
            const int to_station = approach[color][slime[id].p.r][slime[id].p.c];
            if (to_station < 0 || delivery[color] < 0 || direct[id] < 0) continue;
            const int detour = to_station + delivery[color] - direct[id];
            const int saved = max(0, min(5, run[color]) - 1);
            if (saved > detour) gain.push_back({saved - detour, id});
        }
        sort(gain.rbegin(), gain.rend());

        StationSpec spec;
        spec.p = station;
        for (int id : base_id) spec.base.push_back(slime[id]);
        vector<bool> user_color(colors, false);
        int total_gain = 0;
        for (int i = 0; i < static_cast<int>(gain.size()) && i < 32; ++i) {
            const int id = gain[i].second;
            spec.users.push_back(slime[id]);
            user_color[slime[id].color] = true;
            total_gain += gain[i].first;
        }
        if (spec.users.size() < 2 || count(user_color.begin(), user_color.end(), true) < 2) continue;
        spec.proxy_score = total_gain - base_cost;
        result.push_back(move(spec));
    }
    sort(result.begin(), result.end(), [](const StationSpec& a, const StationSpec& b) {
        return a.proxy_score > b.proxy_score;
    });
    return result;
}

optional<vector<Move>> station_solution(const Board& initial, const StationSpec& spec,
                                        const vector<Point>& nests) {
    Planner planner{initial, {}, true};
    for (int i = 1; i < static_cast<int>(spec.base.size()); ++i) {
        const SlimeRef& slime = spec.base[i];
        if (!planner.move_group(slime.p, spec.p, slime.color, false, nests[slime.color], 1)) return nullopt;
    }
    for (const SlimeRef& slime : spec.users) {
        if (!planner.move_group(slime.p, spec.p, slime.color, false, nests[slime.color], 1)) return nullopt;
        if (!planner.move_group(spec.p, nests[slime.color], slime.color, false, {-1, -1}, 1)) return nullopt;
    }
    if (!planner.finish(nests)) return nullopt;
    return planner.moves;
}

bool replay_is_complete(const Board& initial, const vector<Move>& moves) {
    if (static_cast<int>(moves.size()) > MAX_ACTIONS) return false;
    Board replay = initial;
    for (const Move& move : moves) if (!replay.apply(move)) return false;
    return replay.empty();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;
    vector<string> terrain(n);
    for (string& row : terrain) cin >> row;

    Board initial{n, terrain, vector<vector<int>>(n, vector<int>(n, -1)),
                  vector<vector<vector<int>>>(n, vector<vector<int>>(n))};
    vector<Point> nests(k);
    vector<SlimeRef> slime;
    for (int r = 0; r < n; ++r) for (int c = 0; c < n; ++c) {
        const unsigned char ch = static_cast<unsigned char>(terrain[r][c]);
        if ('a' <= ch && ch <= 'z') {
            const int color = ch - 'a';
            initial.stack_at[r][c].push_back(color);
            slime.push_back({{r, c}, color});
        } else if ('A' <= ch && ch <= 'Z') {
            const int color = ch - 'A';
            initial.nest[r][c] = color;
            nests[color] = {r, c};
        }
    }

    vector<Move> best;
    const vector<StationSpec> stations = station_specs(initial, slime, nests, k);
    for (int i = 0; i < static_cast<int>(stations.size()) && i < 8; ++i) {
        optional<vector<Move>> candidate = station_solution(initial, stations[i], nests);
        if (!candidate || !replay_is_complete(initial, *candidate)) continue;
        if (best.empty() || candidate->size() < best.size()) best = move(*candidate);
    }

    if (best.empty()) {
        Planner fallback{initial, {}, true};
        if (fallback.finish(nests) && replay_is_complete(initial, fallback.moves)) best = move(fallback.moves);
    }
    for (const Move& move : best) {
        cout << move.r << ' ' << move.c << ' ' << move.keep << ' ' << DIR[move.dir] << ' ' << move.len << '\n';
    }
}
