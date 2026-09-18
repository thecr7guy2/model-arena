#include <atomic>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

static constexpr int FRAME_MS = 45;

static std::atomic<bool> g_running{true};

static void sig_handler(int) { g_running = false; }

struct Cell {
    char c = ' ';
    unsigned char col = 0;
    bool operator!=(const Cell& o) const { return c != o.c || col != o.col; }
};

static Cell EMPTY_CELL;

struct Species {
    std::vector<std::string> right;
    int color = 37;
    float vmin = 0.5f, vmax = 1.5f;
    float bobAmp = 1.0f, bobSpd = 1.0f;
};

struct Fish {
    const Species* sp;
    double x, y0, vx, ph;
};

struct Bubble {
    double x0, y, spd, ph, size;
};

struct Weed {
    int x, height;
    double phase, speed, amp;
    int color;
};

struct Chest {
    int x;
    bool open;
    double timer;
};

static char mirror_char(char c) {
    switch (c) {
        case '<': return '>';
        case '>': return '<';
        case '(': return ')';
        case ')': return '(';
        case '/': return '\\';
        case '\\': return '/';
        case '{': return '}';
        case '}': return '{';
        case '[': return ']';
        case ']': return '[';
        case '_': return '_';
        case '-': return '-';
        default: return c;
    }
}

static std::vector<std::string> mirrored(const Species& s) {
    std::vector<std::string> out = s.right;
    for (auto& row : out) {
        std::reverse(row.begin(), row.end());
        for (auto& ch : row) ch = mirror_char(ch);
    }
    return out;
}

static Species SPECIES[5] = {
    { { "><(('>" }, 96, 1.8f, 2.6f, 0.8f, 5.5f },
    { { "><(((((*>" }, 93, 0.9f, 1.4f, 1.2f, 3.0f },
    { { ">}==<((((@>" }, 35, 0.5f, 0.8f, 1.0f, 2.2f },
    { { "<('o')>" }, 92, 0.4f, 0.7f, 2.2f, 4.0f },
    { { ">+==<<{{(O>" }, 90, 0.35f, 0.55f, 0.6f, 1.4f },
};

static Chest CHEST;
static std::vector<Fish> FISHES;
static std::vector<Bubble> BUBBLES;
static std::vector<Weed> WEEDS;
static std::mt19937 RNG{42};

static double urand(double lo, double hi) {
    std::uniform_real_distribution<double> d(lo, hi);
    return d(RNG);
}

static int W = 80, H = 24;
static std::vector<Cell> GRID, PREV;

static void put(int x, int y, char c, unsigned char col) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    GRID[y * W + x].c = c;
    GRID[y * W + x].col = col;
}

static void put_sprite(std::vector<std::string> rows, int cx, int cy, int col) {
    int h = (int)rows.size();
    int w = 0;
    for (auto& r : rows) w = std::max(w, (int)r.size());
    int top = cy - h / 2;
    int left = cx - w / 2;
    for (int i = 0; i < h; i++) {
        const std::string& r = rows[i];
        for (size_t j = 0; j < r.size(); j++)
            if (r[j] != ' ') put(left + (int)j, top + i, r[j], (unsigned char)col);
    }
}

static void put_base(std::vector<std::string> rows, int bx, int baseY, int col) {
    int w = 0;
    for (auto& r : rows) w = std::max(w, (int)r.size());
    for (int i = 0; i < (int)rows.size(); i++) {
        const std::string& r = rows[i];
        for (size_t j = 0; j < r.size(); j++)
            if (r[j] != ' ') put(bx + (int)j, baseY - ((int)rows.size() - 1) + i, r[j], (unsigned char)col);
    }
    (void)w;
}

static void layout_deco() {
    WEEDS.clear();
    int seabed = H - 2;
    int n = std::max(3, W / 16);
    int chestZone = CHEST.x - 2;
    for (int i = 0; i < n; i++) {
        int wx;
        int tries = 0;
        do {
            wx = 2 + (int)urand(1, W - 3);
            tries++;
        } while (tries < 10 && std::abs(wx - chestZone) < 8);
        int hgt = std::max(3, (int)urand(seabed * 0.25, seabed * 0.55));
        WEEDS.push_back({wx, hgt, urand(0, 6.28), urand(0.8, 2.0), urand(0.8, 2.2),
                         (int)(urand(0, 1) < 0.5 ? 32 : 92)});
        WEEDS.back().phase = urand(0, 6.28);
    }
}

static void resize_world(bool fresh) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        W = std::max(40, (int)ws.ws_col);
        H = std::max(16, (int)ws.ws_row);
    }
    GRID.assign(W * H, EMPTY_CELL);
    if (fresh) {
        CHEST = {std::max(4, W / 5), false, urand(2.5, 6.0)};
        FISHES.clear();
        int seabed = H - 2;
        for (int si = 0; si < 5; si++) {
            int count = (si == 0) ? 5 : (si == 4 ? 2 : 3);
            for (int k = 0; k < count; k++) {
                Fish f;
                f.sp = &SPECIES[si];
                f.vx = urand(SPECIES[si].vmin, SPECIES[si].vmax) *
                       (urand(0, 1) < 0.5 ? -1.0 : 1.0);
                f.x = urand(2, W - 2);
                f.y0 = urand(3.5, std::max(4.0, (double)seabed - 2.0));
                f.ph = urand(0, 6.28);
                FISHES.push_back(f);
            }
        }
    } else {
        for (auto& f : FISHES) {
            f.x = std::clamp(f.x, 2.0, (double)W - 3);
            f.y0 = std::clamp(f.y0, 3.5, (double)H - 4.5);
        }
    }
    CHEST.x = std::clamp(CHEST.x, 2, W / 2);
    layout_deco();
    PREV.assign(W * H, Cell{'\xff', 255});
    std::printf("\x1b[2J");
}

static void spawn_bubbles(double t) {
    if (BUBBLES.size() < 60) {
        if (urand(0, 1) < 0.10) {
            Bubble b;
            b.x0 = urand(1, W - 2);
            b.y = H - 2.5;
            b.spd = urand(0.35, 0.8);
            b.ph = urand(0, 6.28);
            b.size = urand(0, 1);
            BUBBLES.push_back(b);
        }
        if (CHEST.open && urand(0, 1) < 0.5) {
            Bubble b;
            b.x0 = CHEST.x + urand(-2, 8);
            b.y = H - 3.5;
            b.spd = urand(0.5, 1.0);
            b.ph = urand(0, 6.28);
            b.size = urand(0, 1);
            BUBBLES.push_back(b);
        }
    }
    for (auto it = BUBBLES.begin(); it != BUBBLES.end();) {
        it->y -= it->spd * (FRAME_MS / 1000.0);
        if (it->y < 1.5) it = BUBBLES.erase(it);
        else ++it;
    }
    (void)t;
}

static void draw_chest(int seabed, double t) {
    static const std::vector<std::string> closedR = {
        " ,______,",
        "|  (--)  |",
        "|___[]___|"};
    static const std::vector<std::string> openR = {
        " ,______,",
        " ) ~~~~ (",
        "| *$ %*$* |",
        "|_________|"};
    put_base(CHEST.open ? openR : closedR, CHEST.x, seabed, CHEST.open ? 93 : 33);
    if (CHEST.open) {
        static const char sparks[] = {'*', '+', 'x', '*', '.', '+'};
        for (int i = 0; i < 6; i++) {
            int sx = CHEST.x - 2 + (i * 2) % 12;
            int sy = seabed - 4 - (i % 3);
            double blink = sin(t * 4.0 + i * 1.7);
            if (blink > 0.2) put(sx, sy, sparks[i], 93);
        }
    }
}

static void draw_weeds(double t) {
    int seabed = H - 2;
    for (const auto& w : WEEDS) {
        for (int i = 0; i < w.height; i++) {
            int y = seabed - i;
            double frac = (double)i / w.height;
            double sway = w.amp * frac * sin(t * w.speed + w.phase + i * 0.55);
            char c;
            double a = fabs(sway);
            if (a < 0.35) c = '|';
            else if (a < 0.85) c = (sway > 0) ? '\'' : '`';
            else c = (sway > 0) ? ')' : '(';
            put(w.x + (int)round(sway * 0.7), y, c, (unsigned char)w.color);
        }
    }
}

static void draw_frame(double t) {
    GRID.assign(W * H, EMPTY_CELL);
    int seabed = H - 2;
    int shift = (int)(t * 4.0) % 97;
    for (int x = 0; x < W; x++) {
        int k = (x + shift) % 11;
        if (k == 0 || k == 1) put(x, 0, '~', 36);
        else if (((x * 5 + shift * 3) % 17) == 0) put(x, 1, '~', 36);
    }
    for (int x = 0; x < W; x++) {
        unsigned hsh = (unsigned)(x * 2654435761u);
        put(x, seabed, ".',`,"[(hsh >> 3) % 5], 33);
        put(x, seabed + 1, ",._'_-"[(hsh >> 7) % 6], 33);
    }
    draw_weeds(t);
    draw_chest(seabed, t);
    for (const auto& b : BUBBLES) {
        int bx = (int)round(b.x0 + sin(b.y * 0.55 + b.ph) * (0.6 + b.size));
        char bc = b.size > 0.75 ? 'O' : (b.size > 0.4 ? 'o' : '.');
        put(bx, (int)b.y, bc, 94);
    }
    for (const auto& f : FISHES) {
        const Species& sp = *f.sp;
        std::vector<std::string> art = (f.vx >= 0) ? sp.right : mirrored(sp);
        int fy = (int)round(f.y0 + sin(t * sp.bobSpd + f.ph) * sp.bobAmp);
        fy = std::clamp(fy, 2, seabed - 1);
        put_sprite(art, (int)f.x, fy, (unsigned char)sp.color);
    }
}

static std::string render_diff() {
    std::string out;
    char buf[64];
    int lastCol = -1;
    bool any = false;
    out.reserve(GRID.size() * 4 + 64);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W;) {
            int idx = y * W + x;
            if (!(GRID[idx] != PREV[idx])) { x++; continue; }
            std::snprintf(buf, sizeof buf, "\x1b[%d;%dH", y + 1, x + 1);
            out += buf;
            any = true;
            while (x < W && GRID[y * W + x] != PREV[y * W + x]) {
                const Cell& c = GRID[y * W + x];
                int col = c.col;
                if (col != lastCol) {
                    if (col == 0) out += "\x1b[m";
                    else out += "\x1b[" + std::to_string(col) + "m";
                    lastCol = col;
                }
                if (c.c == '\0') out += ' ';
                else out += c.c;
                PREV[y * W + x] = c;
                x++;
            }
        }
    }
    if (any) {
        out += "\x1b[m";
        return out;
    }
    return "";
}

static void init_terminal() {
    std::printf("\x1b[?1049h\x1b[?25l");
    std::fflush(stdout);
    termios t{};
    if (tcgetattr(STDIN_FILENO, &t) == 0) {
        t.c_lflag &= ~(ICANON | ECHO | ISIG);
        t.c_iflag &= ~(IXON | ICRNL);
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
}

static termios g_saved;
static bool g_savedValid = false;

static void restore_terminal() {
    if (g_savedValid) tcsetattr(STDIN_FILENO, TCSANOW, &g_saved);
    std::printf("\x1b[m\x1b[?25h\x1b[2J\x1b[?1049l");
    std::fflush(stdout);
}

static void input_fn() {
    bool eofSeen = false;
    while (g_running.load()) {
        if (eofSeen) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            continue;
        }
        pollfd p{STDIN_FILENO, POLLIN, 0};
        int r = poll(&p, 1, 120);
        if (r > 0 && (p.revents & POLLIN)) {
            char c;
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if (n == 1) {
                if (c == 'q' || c == 'Q' || c == 3 || c == 4 || c == 27) g_running = false;
            } else if (n == 0) {
                eofSeen = true;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
}

int main() {
    if (tcgetattr(STDIN_FILENO, &g_saved) == 0) g_savedValid = true;
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);
    RNG.seed((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());

    init_terminal();
    resize_world(true);

    std::thread inputThread(input_fn);
    auto next = std::chrono::steady_clock::now();

    while (g_running.load()) {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
            (std::max(40, (int)ws.ws_col) != W || std::max(16, (int)ws.ws_row) != H)) {
            resize_world(false);
        }
        double t = std::chrono::duration<double>(
                       std::chrono::steady_clock::now().time_since_epoch())
                       .count();

        CHEST.timer -= FRAME_MS / 1000.0;
        if (CHEST.timer <= 0) {
            CHEST.open = !CHEST.open;
            CHEST.timer = CHEST.open ? 3.5 : urand(5.0, 11.0);
        }

        for (auto& f : FISHES) {
            f.x += f.vx * (FRAME_MS / 1000.0);
            int wHalf = 4;
            if (f.x < 1 + wHalf) { f.x = 1 + wHalf; f.vx = std::abs(f.vx); }
            if (f.x > W - 1 - wHalf) { f.x = W - 1 - wHalf; f.vx = -std::abs(f.vx); }
            (void)f;
        }

        spawn_bubbles(t);
        draw_frame(t);
        std::string s = render_diff();
        if (!s.empty()) fwrite(s.data(), 1, s.size(), stdout);
        fflush(stdout);

        next += std::chrono::milliseconds(FRAME_MS);
        std::this_thread::sleep_until(next);
    }

    inputThread.join();
    restore_terminal();
    std::printf("The aquarium is closed. Come back soon!\n");
    return 0;
}
