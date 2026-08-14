// Animated ASCII-art aquarium for the terminal.
// Build:  g++ -std=c++17 -O2 -pthread aquarium.cpp -o aquarium
// Press 'q' to quit cleanly.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace {

constexpr double kPi = 3.14159265358979323846;

std::atomic<bool> g_quit(false);

std::mt19937 g_rng(static_cast<unsigned>(
    steady_clock::now().time_since_epoch().count()));

double rand01() { return std::uniform_real_distribution<double>(0.0, 1.0)(g_rng); }
int    randi(int a, int b) { return std::uniform_int_distribution<int>(a, b)(g_rng); }
double randd(double a, double b) { return std::uniform_real_distribution<double>(a, b)(g_rng); }
int    clamp(int v, int lo, int hi) { return std::max(lo, std::min(v, hi)); }

// ---------------------------------------------------------------------------
// Fish species: colour, speed range (px/s), art facing right / facing left.
// ---------------------------------------------------------------------------
struct Species {
    int         fg;        // ANSI foreground code (30..37, 90..97)
    double      speedMin, speedMax;
    std::string right, left;

    int width(int dir) const { return static_cast<int>((dir > 0 ? right : left).size()); }
    const std::string& art(int dir) const { return (dir > 0) ? right : left; }
};

const std::vector<Species> kSpecies = {
    {36, 45.0, 75.0,  "><(((*>",     "<*)))><"},       // guppy
    {33, 30.0, 55.0,  "><((((*>",    "<*))))><"},      // goldfish
    {35, 20.0, 40.0,  "(o.o)><",     "><(o.o)"},       // puffer (symmetric face)
    {32, 70.0, 110.0, "~~~<(((((*>", "<*))))>~~~"}     // electric eel
};

// ---------------------------------------------------------------------------
// Treasure chest: closed / half-open / fully open.
// ---------------------------------------------------------------------------
const std::vector<std::string> kChestClosed = {
    " /====\\ ",
    " | $ $ | ",
    " | $ $ | ",
    " |     | ",
    " \\____/ "
};
const std::vector<std::string> kChestHalf = {
    "  /==\\  ",
    " /    \\ ",
    " | $ $ | ",
    " |  $  | ",
    " \\____/ "
};
const std::vector<std::string> kChestOpen = {
    "  /==\\   ",
    " /    \\  ",
    " |$ $ $| ",
    " |  $  | ",
    " \\_____/ "
};

enum ChestState { CS_CLOSED, CS_OPENING, CS_OPEN, CS_CLOSING };

struct Fish {
    int    sp;
    int    dir;      // +1 right, -1 left
    double x, y;     // x = left edge (px), y = centre row
    double vx;       // px/s
    double turnCd;   // seconds until the fish may change its mind
    double bobPhase, bobSpeed;
};

struct Bubble {
    double x, y;            // base position (px)
    double vy;              // px/s upward
    double wobAmp, wobFreq, phase;
};

struct Strand {              // one seaweed stalk
    int    x;                // base column
    int    len;              // number of segments above the floor
    double phase, speed, amp;
};

// ---------------------------------------------------------------------------
// Terminal helper: put stdin in raw-ish mode and use the alternate screen.
// ---------------------------------------------------------------------------
class TerminalGuard {
    bool         ok;
    struct termios saved;

public:
    TerminalGuard() {
        ok = (isatty(STDIN_FILENO) != 0);
        if (ok && tcgetattr(STDIN_FILENO, &saved) == 0) {
            struct termios raw = saved;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN]  = 1;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        }
        std::printf("\x1b[?1049h\x1b[H\x1b[2J\x1b[?25l");
        std::fflush(stdout);
    }
    ~TerminalGuard() {
        std::printf("\x1b[0m\x1b[?25h\x1b[?1049l");
        std::fflush(stdout);
        if (ok) tcsetattr(STDIN_FILENO, TCSANOW, &saved);
    }
};

bool getTermSize(int& w, int& h) {
    struct winsize ws{};
    if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
        ws.ws_col > 0 && ws.ws_row > 0) {
        w = ws.ws_col;
        h = ws.ws_row;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Aquarium
// ---------------------------------------------------------------------------
class Aquarium {
    int W = 80, H = 24;

    std::vector<std::string>   cells;          // char per cell
    std::vector<std::vector<int>> fg;          // ANSI fg code per cell (0 = default)

    std::vector<Fish>   fish;
    std::vector<Bubble> bubbles;
    std::vector<Strand> strands;

    int           chestX = 0;
    ChestState    chestState = CS_CLOSED;
    double        chestNextT = 0.0;
    std::vector<std::pair<int, int>> sparkles;   // gold sparkle positions
    double        phase0 = 0.0;

    double globalT = 0.0;

public:
    void run();

private:
    void resize(int w, int h);
    void newFish(int sp);
    void update(double dt);
    void draw();
    void render();
    void flipFish(Fish& f);
    void putPx(int x, int y, int c, char ch);
    void blank();
    void drawSurface();
    void drawSand();
    void drawSeaweed();
    void drawChest();
    void drawFish();
    void drawBubbles();
    void drawSparkles();
    void drawHelp();
};

void Aquarium::putPx(int x, int y, int c, char ch) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    cells[y][x] = ch;
    fg[y][x]    = c;
}

void Aquarium::flipFish(Fish& f) {
    int wd = kSpecies[f.sp].width(f.dir);
    f.dir = -f.dir;
    f.vx  = -f.vx;
    int wn = kSpecies[f.sp].width(f.dir);
    f.x  += (wd - wn) / 2.0;   // keep the fish roughly centred on the turn
}

void Aquarium::newFish(int sp) {
    Fish f;
    f.sp  = sp;
    f.dir = (rand01() < 0.5) ? 1 : -1;
    const Species& s = kSpecies[sp];
    int wd          = s.width(f.dir);
    if (f.dir > 0) f.x = -double(wd) - randd(0.0, 12.0);   // swim in from the left
    else           f.x = double(W) + randd(0.0, 12.0);     // ...or from the right
    f.y = randd(1.5, std::max(2.0, double(H) - 3.0));
    f.vx         = f.dir * randd(s.speedMin, s.speedMax);
    f.turnCd     = randd(1.2, 4.5);
    f.bobPhase   = randd(0.0, 2.0 * kPi);
    f.bobSpeed   = randd(0.4, 1.1);
    fish.push_back(f);
}

void Aquarium::resize(int w, int h) {
    W = std::max(10, w);
    H = std::max(6, h);

    cells.assign(H, std::string(static_cast<size_t>(W), ' '));
    fg.assign(static_cast<size_t>(H), std::vector<int>(static_cast<size_t>(W), 0));

    // --- fish ---
    int n = clamp(static_cast<int>((long)W * H / 200), 8, 26);
    fish.clear();
    for (size_t i = 0; i < kSpecies.size(); ++i) newFish(static_cast<int>(i));
    for (int i = static_cast<int>(kSpecies.size()); i < n; ++i)
        newFish(randi(0, static_cast<int>(kSpecies.size()) - 1));

    // --- bubbles ---
    int nb = clamp(static_cast<int>(std::llround(W * 0.18)), 8, 18);
    bubbles.clear();
    for (int i = 0; i < nb; ++i) {
        Bubble b;
        b.x      = randd(1.0, W - 2.0);
        b.y      = randd(H - 6.0, H - 2.0);
        b.vy     = randd(18.0, 40.0);
        b.wobAmp = randd(0.4, 1.2);
        b.wobFreq = randd(2.0, 4.0);
        b.phase  = randd(0.0, 2.0 * kPi);
        bubbles.push_back(b);
    }

    // --- seaweed ---
    int ns = clamp(static_cast<int>(std::llround(W / 9.0)), 4, 10);
    strands.clear();
    for (int i = 0; i < ns; ++i) {
        Strand st;
        st.x    = clamp(randi(0, W - 1), 0, W - 1);
        st.len  = std::max(4, std::min(12, H / 5));
        st.phase = randd(0.0, 2.0 * kPi);
        st.speed = randd(0.4, 0.9);
        st.amp  = randd(2.0, 5.0);
        strands.push_back(st);
    }

    // --- treasure chest ---
    chestX = static_cast<int>(std::llround(W * 0.52));
    chestState = CS_CLOSED;
    chestNextT = globalT + randd(1.5, 3.5);   // open quickly the first time
    sparkles.clear();
    for (int i = 0; i < 6; ++i)
        sparkles.push_back({randi(-2, 10), randi(-5, 4)});
    phase0 = randd(0.0, 2.0 * kPi);
}

void Aquarium::update(double dt) {
    // fish
    for (Fish& f : fish) {
        f.turnCd -= dt;
        if (f.turnCd <= 0.0) {
            f.turnCd = randd(1.5, 5.0);
            if (rand01() < 0.30) flipFish(f);   // a fish changes its mind
        }
        f.x += f.vx * dt;
        int wd = kSpecies[f.sp].width(f.dir);
        if (f.vx > 0 && f.x + wd > W - 1.0) { f.x = W - 1.0 - wd; flipFish(f); }
        else if (f.vx < 0 && f.x < 0.0)     { f.x = 0.0;           flipFish(f); }
    }

    // bubbles
    for (Bubble& b : bubbles) {
        b.y -= b.vy * dt;
        if (b.y < 0.0) {
            b.x = randd(1.0, W - 2.0);
            b.y = randd(H - 6.0, H - 2.0);
            b.vy = randd(18.0, 40.0);
            b.wobAmp = randd(0.4, 1.2);
            b.wobFreq = randd(2.0, 4.0);
            b.phase = randd(0.0, 2.0 * kPi);
        }
    }

    // treasure chest state machine
    switch (chestState) {
        case CS_CLOSED:
            if (globalT >= chestNextT) {
                chestState = CS_OPENING;
                chestNextT = globalT + 0.35;
            }
            break;
        case CS_OPENING:
            if (globalT >= chestNextT) {
                chestState = CS_OPEN;
                chestNextT = globalT + randd(3.0, 6.0);
            }
            break;
        case CS_OPEN:
            if (globalT >= chestNextT) {
                chestState = CS_CLOSING;
                chestNextT = globalT + 0.35;
            }
            break;
        case CS_CLOSING:
            if (globalT >= chestNextT) {
                chestState = CS_CLOSED;
                chestNextT = globalT + randd(6.0, 14.0);
            }
            break;
    }
}

void Aquarium::blank() {
    for (auto& row : cells) std::fill(row.begin(), row.end(), ' ');
    for (auto& row : fg)    std::fill(row.begin(), row.end(), 0);
}

void Aquarium::drawSurface() {
    for (int x = 0; x < W; ++x) {
        double v = std::sin(globalT * 2.0 + x * 0.55);
        putPx(x, 0, 36, v > 0.72 ? '~' : ' ');
    }
}

void Aquarium::drawSand() {
    static const char* pat = "_~.~*~.^~";
    for (int x = 0; x < W; ++x) {
        int h = (x * 37 + (x % 13) * 7 + static_cast<int>(globalT * 4) % 3) % 10;
        putPx(x, H - 1, 33, pat[h]);
    }
}

void Aquarium::drawSeaweed() {
    const int floorY = H - 1;
    for (const Strand& st : strands) {
        for (int k = 0; k < st.len; ++k) {
            double a = static_cast<double>(k + 1) / st.len;
            double off1 = st.amp * std::sin(globalT * st.speed + st.phase + k * 0.8) * a;
            double off0 = (k == 0)
                ? 0.0
                : st.amp * std::sin(globalT * st.speed + st.phase + (k - 1) * 0.8)
                    * (static_cast<double>(k) / st.len);
            int o = static_cast<int>(std::lround(off1));
            int po = (k == 0) ? 0 : static_cast<int>(std::lround(off0));
            char ch;
            if      (o > po) ch = '/';
            else if (o < po) ch = '\\';
            else             ch = (k % 2) ? '|' : '~';
            putPx(st.x + o, floorY - 1 - k, 92, ch);
        }
    }
}

void Aquarium::drawChest() {
    const int floorY = H - 1;
    const int bottomY = floorY - 1;

    const std::vector<std::string>* art = nullptr;
    switch (chestState) {
        case CS_CLOSED:  art = &kChestClosed; break;
        case CS_OPENING: art = &kChestHalf;   break;
        case CS_OPEN:    art = &kChestOpen;   break;
        case CS_CLOSING: art = &kChestHalf;   break;
    }

    int artW = 0;
    for (const auto& row : *art) artW = std::max(artW, static_cast<int>(row.size()));
    int cx = chestX - artW / 2;
    int n  = static_cast<int>(art->size());
    int topY = bottomY - (n - 1);

    for (int r = 0; r < n; ++r) {
        const std::string& row = (*art)[r];
        for (size_t i = 0; i < row.size(); ++i) {
            char ch = row[i];
            if (ch == ' ') continue;
            putPx(cx + static_cast<int>(i), topY + r, ch == '$' ? 33 : 37, ch);
        }
    }
    if (chestState == CS_OPEN) drawSparkles();
}

void Aquarium::drawSparkles() {
    for (size_t i = 0; i < sparkles.size(); ++i) {
        double fl = std::sin(globalT * (5.0 + i) + sparkles[i].first + phase0);
        if (fl < 0.4) continue;
        int x = chestX + sparkles[i].first;
        int y = H - 2 + sparkles[i].second;
        if (y < 0 || y > H - 3) y = clamp(y, 1, H - 3);
        putPx(x, y, 33, (fl > 0.85) ? '*' : '+');
    }
}

void Aquarium::drawFish() {
    for (const Fish& f : fish) {
        const Species& s = kSpecies[f.sp];
        const std::string& art = s.art(f.dir);
        int dy = clamp(static_cast<int>(
            std::lround(f.y + 0.6 * std::sin(globalT * f.bobSpeed + f.bobPhase))), 1, H - 2);
        int dx = static_cast<int>(std::lround(f.x));
        for (size_t i = 0; i < art.size(); ++i) {
            char ch = art[i];
            if (ch == ' ') continue;
            putPx(dx + static_cast<int>(i), dy, s.fg, ch);
        }
    }
}

void Aquarium::drawBubbles() {
    for (const Bubble& b : bubbles) {
        int x = static_cast<int>(std::lround(b.x + b.wobAmp * std::sin(globalT * b.wobFreq + b.phase)));
        int y = static_cast<int>(std::lround(b.y));
        if (y < 0 || y >= H - 1) continue;
        char ch = ((x * 7 + y * 13 + static_cast<int>(globalT * 20)) % 3 == 0) ? 'O' : 'o';
        putPx(x, y, 37, ch);
        putPx(x - 1, y + 1, 37, '.');
    }
}

void Aquarium::drawHelp() {
    const std::string msg = "q:quit";
    for (size_t i = 0; i < msg.size(); ++i)
        putPx(2 + static_cast<int>(i), H - 2, 90, msg[i]);
}

void Aquarium::draw() {
    blank();
    drawSurface();
    drawSand();
    drawSeaweed();
    drawChest();
    drawFish();
    drawBubbles();
    drawHelp();
}

void Aquarium::render() {
    std::string out;
    out.reserve(static_cast<size_t>(W) * H * 3 + 64);
    out += "\x1b[H";
    int last = -1;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int c = fg[y][x];
            if (c != last) {
                out += "\x1b[0;44m";          // reset, blue water background
                if (c != 0) {
                    char b[16];
                    std::snprintf(b, sizeof b, "\x1b[%dm", c);
                    out += b;
                }
                last = c;
            }
            out += cells[y][x];
        }
    }
    std::fwrite(out.data(), 1, out.size(), stdout);
    std::fflush(stdout);
}

void Aquarium::run() {
    int tw = 80, th = 24;
    getTermSize(tw, th);
    resize(tw, th);

    auto t0 = steady_clock::now();
    auto prev = t0;
    auto next = t0 + 33ms;

    while (!g_quit) {
        auto now = steady_clock::now();
        double dt = duration<double>(now - prev).count();
        prev = now;
        if (dt > 0.25) dt = 0.25;
        globalT += dt;

        int tw2 = 0, th2 = 0;
        if (getTermSize(tw2, th2) && (tw2 != W || th2 != H)) resize(tw2, th2);

        update(dt);
        draw();
        render();

        next += 33ms;
        if (now > next + 33ms) next = now;
        std::this_thread::sleep_until(next);
    }
}

// Blocks reading stdin; any 'q'/'Q' requests a clean exit.
void inputLoop() {
    while (!g_quit) {
        unsigned char c = 0;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1) {
            if (c == 'q' || c == 'Q') g_quit = true;
        } else {
            // EOF / EAGAIN on a non-tty stdin: don't spin.
            std::this_thread::sleep_for(10ms);
        }
    }
}

} // namespace

int main() {
    TerminalGuard guard;
    std::thread input(inputLoop);
    {
        Aquarium aquarium;
        aquarium.run();
    }
    input.join();
    std::printf("\n");
    return 0;
}
