#include <bits/stdc++.h>
using namespace std;

//purely by GPT-5.6-Terra high
//in needs of system thinking (feature hierarchy sort-of)

constexpr int MAX_ACTIONS = 100000;
constexpr int MAX_HEIGHT = 8;
constexpr int DR[4] = {0, 1, 0, -1};
constexpr int DC[4] = {1, 0, -1, 0};
constexpr char DIR[4] = {'R', 'D', 'L', 'U'};

struct Point {
    int r, c;
    bool operator==(const Point& other) const { return r == other.r && c == other.c; }
};

struct Move {
    int r, c, keep, dir, len;
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
        int count = 0;
        while (count < static_cast<int>(s.size()) && s[s.size() - 1 - count] == color) ++count;
        return count;
    }

    void homecoming(Point p) {
        const int color = nest[p.r][p.c];
        if (color < 0) return;
        auto& s = stack_at[p.r][p.c];
        while (!s.empty() && s.back() == color) s.pop_back();
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

vector<vector<int>> distance_field(const Board& board, Point start) {
    vector<vector<int>> dist(board.n, vector<int>(board.n, -1));
    queue<Point> q;
    dist[start.r][start.c] = 0;
    q.push(start);
    while (!q.empty()) {
        Point cur = q.front();
        q.pop();
        for (int d = 0; d < 4; ++d) {
            Point next{cur.r + DR[d], cur.c + DC[d]};
            if (!board.floor(next) || dist[next.r][next.c] != -1) continue;
            dist[next.r][next.c] = dist[cur.r][cur.c] + 1;
            q.push(next);
        }
    }
    return dist;
}

long long potential(const Board& board, const vector<vector<vector<int>>>& dist) {
    long long total = 0;
    for (int r = 0; r < board.n; ++r) for (int c = 0; c < board.n; ++c) {
        for (int color : board.stack_at[r][c]) total += dist[color][r][c];
    }
    return total;
}

struct Candidate {
    Move move;
    long long value;
};

// Every candidate satisfies the hard invariant
// distance[color][landing] = distance[color][source] - jump_length.
// Therefore the travelling monochromatic block strictly lowers the exact global
// potential by block_size * jump_length before any additional homecoming benefit.
Candidate choose_move(const Board& board, const vector<vector<vector<int>>>& dist, mt19937& rng) {
    Candidate best{{-1, -1, -1, -1, -1}, numeric_limits<long long>::min()};

    auto consider = [&](Point from, int color, int run, int count, int dir, int len, Point to) {
        const int keep = board.height(from) - count;
        const int progress = count * len;
        const int merge = board.top_run(to, color);
        const bool mixed_landing = board.height(to) > 0 && board.top_color(to) != color;
        const bool split = count < run;
        const int crowding = max(0, board.height(to) + count - 5);

        // Potential progress has a lexicographically dominant weight.  The
        // remaining terms only distinguish moves with comparable certain gain:
        // merging and retained launch support are useful; splitting, mixed
        // resting, and near-full landings are deviations from that ideal.
        long long value = 10000LL * progress;
        value += 220LL * merge;
        value += 25LL * min(keep, 4);
        value -= 320LL * mixed_landing;
        value -= 180LL * split;
        value -= 25LL * crowding;

        // Randomness is smaller than one unit of certain progress.  Every trial
        // explores a different tie-breaking order without admitting a bad move.
        value += static_cast<int>(rng() % 1201) - 600;
        if (value > best.value) best = {{from.r, from.c, keep, dir, len}, value};
    };

    for (int r = 0; r < board.n; ++r) for (int c = 0; c < board.n; ++c) {
        Point from{r, c};
        const int color = board.top_color(from);
        if (color < 0) continue;
        const int current_distance = dist[color][r][c];
        if (current_distance <= 0) continue;
        const int run = board.top_run(from, color);

        array<int, 2> sizes{run, 1};
        const int options = run == 1 ? 1 : 2;
        for (int option = 0; option < options; ++option) {
            const int count = sizes[option];
            const int keep = board.height(from) - count;
            for (int dir = 0; dir < 4; ++dir) {
                Point to = from;
                int longest = -1, merge_length = -1;
                const int limit = min({keep + 1, current_distance, MAX_HEIGHT});
                for (int len = 1; len <= limit; ++len) {
                    to.r += DR[dir];
                    to.c += DC[dir];
                    if (!board.floor(to)) break;
                    if (board.height(to) + count > MAX_HEIGHT) continue;
                    if (dist[color][to.r][to.c] != current_distance - len) continue;
                    longest = len;
                    if (board.top_color(to) == color) merge_length = len;
                }
                if (longest < 0) continue;
                Point landing{from.r + DR[dir] * longest, from.c + DC[dir] * longest};
                consider(from, color, run, count, dir, longest, landing);
                if (merge_length >= 0 && merge_length != longest) {
                    Point merge_landing{from.r + DR[dir] * merge_length, from.c + DC[dir] * merge_length};
                    consider(from, color, run, count, dir, merge_length, merge_landing);
                }
            }
        }
    }
    return best;
}

optional<vector<Move>> run_trial(const Board& initial, const vector<vector<vector<int>>>& dist, uint32_t seed) {
    Board board = initial;
    vector<Move> moves;
    mt19937 rng(seed);
    for (int step = 0; !board.empty() && step < MAX_ACTIONS; ++step) {
        const Candidate candidate = choose_move(board, dist, rng);
        if (candidate.move.r < 0) return nullopt;
        const long long before = potential(board, dist);
        if (!board.apply(candidate.move)) return nullopt;
        const long long after = potential(board, dist);
        if (after >= before) return nullopt;
        moves.push_back(candidate.move);
    }
    if (!board.empty()) return nullopt;
    return moves;
}

bool replay_is_complete(const Board& initial, const vector<Move>& moves) {
    if (static_cast<int>(moves.size()) > MAX_ACTIONS) return false;
    Board board = initial;
    for (const Move& move : moves) if (!board.apply(move)) return false;
    return board.empty();
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
    uint32_t fingerprint = 2166136261u;
    for (int r = 0; r < n; ++r) for (int c = 0; c < n; ++c) {
        const unsigned char ch = static_cast<unsigned char>(terrain[r][c]);
        fingerprint = (fingerprint ^ ch) * 16777619u;
        if ('a' <= ch && ch <= 'z') initial.stack_at[r][c].push_back(ch - 'a');
        else if ('A' <= ch && ch <= 'Z') {
            initial.nest[r][c] = ch - 'A';
            nests[ch - 'A'] = {r, c};
        }
    }

    vector<vector<vector<int>>> dist(k);
    for (int color = 0; color < k; ++color) dist[color] = distance_field(initial, nests[color]);

    vector<Move> best;
    for (uint32_t trial = 0; trial < 4; ++trial) {
        optional<vector<Move>> candidate = run_trial(initial, dist, fingerprint + 0x9e3779b9u * trial);
        if (!candidate || !replay_is_complete(initial, *candidate)) continue;
        if (best.empty() || candidate->size() < best.size()) best = move(*candidate);
    }

    for (const Move& move : best) {
        cout << move.r << ' ' << move.c << ' ' << move.keep << ' ' << DIR[move.dir] << ' ' << move.len << '\n';
    }
}
