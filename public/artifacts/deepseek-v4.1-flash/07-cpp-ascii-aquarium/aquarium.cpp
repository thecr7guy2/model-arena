#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace {

using Clock = std::chrono::steady_clock;

termios g_orig{};
bool g_raw = false;
volatile std::sig_atomic_t g_stop = 0;
bool g_quit = false;

void restoreTerm() {
    if (g_raw) {
        ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig);
        g_raw = false;
    }
    static const char s[] = "\x1b[0m\x1b[?25h\x1b[?1049l";
    ssize_t r = ::write(STDOUT_FILENO, s, sizeof(s) - 1);
    (void)r;
}

void onSignal(int) { g_stop = 1; }

void setupTerm() {
    if (::tcgetattr(STDIN_FILENO, &g_orig) == 0) {
        termios raw = g_orig;
        raw.c_lflag &= ~(ICANON | ECHO | ISIG);
        raw.c_iflag &= ~(IXON | ICRNL);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0) g_raw = true;
    }
    static const char s[] = "\x1b[?1049h\x1b[2J\x1b[?25l";
    ssize_t r = ::write(STDOUT_FILENO, s, sizeof(s) - 1);
    (void)r;
}

void writeAll(const std::string& s) {
    size_t off = 0;
    while (off < s.size()) {
        ssize_t n = ::write(STDOUT_FILENO, s.data() + off, s.size() - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return;
        }
        if (n == 0) return;
        off += size_t(n);
    }
}

void getSize(int& W, int& H) {
    winsize ws{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        W = std::min(400, std::max(1, int(ws.ws_col) - 1));
        H = std::min(160, std::max(1, int(ws.ws_row) - 1));
    } else {
        W = 80;
        H = 24;
    }
}

void pollInput() {
    char buf[256];
    for (;;) {
        ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
        if (n <= 0) break;
        for (ssize_t i = 0; i < n; ++i)
            if (buf[i] == 'q' || buf[i] == 'Q' || buf[i] == 3) g_quit = true;
    }
}

std::mt19937 g_rng(static_cast<std::mt19937::result_type>(
                       std::chrono::high_resolution_clock::now().time_since_epoch().count()) ^
                   static_cast<std::mt19937::result_type>(::getpid()));

float frand(float a, float b) {
    return std::uniform_real_distribution<float>(a, b)(g_rng);
}

int irand(int a, int b) {
    if (b < a) std::swap(a, b);
    return std::uniform_int_distribution<int>(a, b)(g_rng);
}

uint32_t hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

struct Canvas {
    int w = 0, h = 0;
    std::vector<char> ch;
    std::vector<uint8_t> fg, bg;

    void resize(int W, int H) {
        w = W;
        h = H;
        ch.assign(size_t(W) * H, ' ');
        fg.assign(size_t(W) * H, 7);
        bg.assign(size_t(W) * H, 17);
    }

    void put(int x, int y, char c, uint8_t f, uint8_t b) {
        if (x < 0 || y < 0 || x >= w || y >= h) return;
        size_t i = size_t(y) * w + x;
        ch[i] = c;
        fg[i] = f;
        bg[i] = b;
    }

    void text(int x, int y, const std::string& s, uint8_t f, uint8_t b) {
        for (size_t i = 0; i < s.size(); ++i) put(x + int(i), y, s[i], f, b);
    }

    std::string render() const {
        std::string out;
        out.reserve(size_t(w) * h * 4 + size_t(h) * 16);
        int curf = -1, curb = -1;
        for (int y = 0; y < h; ++y) {
            out += "\x1b[";
            out += std::to_string(y + 1);
            out += ";1H";
            for (int x = 0; x < w; ++x) {
                size_t i = size_t(y) * w + x;
                int f = fg[i], b = bg[i];
                if (f != curf || b != curb) {
                    out += "\x1b[38;5;";
                    out += std::to_string(f);
                    out += ";48;5;";
                    out += std::to_string(b);
                    out += 'm';
                    curf = f;
                    curb = b;
                }
                out += ch[i];
            }
        }
        out += "\x1b[0m";
        return out;
    }
};

constexpr uint8_t SAND_BG = 58;

uint8_t waterBG(int y, int h) {
    static const uint8_t pal[10] = {24, 25, 26, 27, 26, 25, 24, 19, 18, 17};
    int i = (h <= 1) ? 0 : y * 10 / h;
    if (i < 0) i = 0;
    if (i > 9) i = 9;
    return pal[i];
}

std::string mirrorArt(const std::string& s) {
    std::string r = s;
    std::reverse(r.begin(), r.end());
    for (char& c : r) {
        switch (c) {
            case '(': c = ')'; break;
            case ')': c = '('; break;
            case '<': c = '>'; break;
            case '>': c = '<'; break;
            case '/': c = '\\'; break;
            case '\\': c = '/'; break;
            case '[': c = ']'; break;
            case ']': c = '['; break;
            case '{': c = '}'; break;
            case '}': c = '{'; break;
            default: break;
        }
    }
    return r;
}

struct Species {
    std::string right;
    std::string left;
    float vmin = 4.0f, vmax = 6.0f;
    uint8_t body = 231, fin = 226;
    int weight = 1;
};

std::vector<Species> g_species;

void addSpecies(const char* art, float vmin, float vmax, uint8_t body, uint8_t fin, int weight) {
    Species s;
    s.right = art;
    s.left = mirrorArt(art);
    s.vmin = vmin;
    s.vmax = vmax;
    s.body = body;
    s.fin = fin;
    s.weight = weight;
    g_species.push_back(s);
}

void initSpecies() {
    addSpecies("><>", 10.0f, 16.0f, 159, 123, 4);
    addSpecies("><((('>", 6.0f, 10.0f, 51, 201, 4);
    addSpecies("><(((o>", 4.0f, 7.0f, 214, 208, 3);
    addSpecies("><(O)>", 2.5f, 4.5f, 220, 178, 2);
    addSpecies("><((*))>", 3.0f, 5.5f, 229, 226, 2);
    addSpecies("><(((((*>", 3.5f, 5.5f, 245, 240, 1);
}

struct Fish {
    int sp = 0;
    float x = 0, y = 0, vy = 0;
    float speed = 5;
    int dir = 1;
    float bobPhase = 0, bobSpeed = 1, bobAmp = 0.5f, dartPhase = 0;
};

struct Bubble {
    float x = 0, y = 0, vy = 3, wob = 0, wamp = 0.5f;
    char ch = 'o';
    uint8_t col = 195;
};

struct Spark {
    float x = 0, y = 0, vy = -2, life = 1, maxlife = 1;
    char ch = '*';
    uint8_t col = 226;
};

struct Seaweed {
    int x = 0, len = 5;
    float phase = 0, tscale = 1;
    uint8_t col = 34;
};

struct World {
    int W = 80, H = 24;
    Canvas cv;
    std::vector<Fish> fish;
    std::vector<Bubble> bubbles;
    std::vector<Spark> sparks;
    std::vector<Seaweed> weed;
    std::vector<int> sandH;
    int chestX = -1;
    bool chestOpen = false;
    double chestTimer = 0;
    float time = 0;
};

Fish makeFish(int W, int H) {
    Fish f;
    int total = 0;
    for (const Species& s : g_species) total += s.weight;
    int r = irand(1, total), acc = 0;
    for (size_t i = 0; i < g_species.size(); ++i) {
        acc += g_species[i].weight;
        if (r <= acc) {
            f.sp = int(i);
            break;
        }
    }
    const Species& s = g_species[size_t(f.sp)];
    int aw = int(s.right.size());
    f.dir = irand(0, 1) ? 1 : -1;
    f.speed = frand(s.vmin, s.vmax);
    f.x = frand(1.0f, std::max(2.0f, float(W - 2 - aw)));
    f.y = frand(1.0f, std::max(2.0f, float(H - 4)));
    f.vy = frand(-0.45f, 0.45f);
    f.bobPhase = frand(0.0f, 6.283f);
    f.bobSpeed = frand(0.6f, 1.8f);
    f.bobAmp = frand(0.2f, 1.0f);
    f.dartPhase = frand(0.0f, 6.283f);
    return f;
}

void buildWorld(World& w, int W, int H) {
    w.W = W;
    w.H = H;
    w.cv.resize(W, H);
    w.sandH.assign(size_t(W), 1);
    for (int x = 0; x < W; ++x)
        w.sandH[size_t(x)] = 1 + ((hash32(uint32_t(x) * 2654435761u + 97u) % 100) < 35 ? 1 : 0);

    w.chestX = -1;
    if (W >= 40 && H >= 12) {
        const int cw = 12;
        if (irand(0, 1) == 0)
            w.chestX = irand(3, std::max(3, W / 3));
        else
            w.chestX = irand(std::max(3, W * 2 / 3), std::max(3, W - cw - 3));
        if (w.chestX + cw > W - 2) w.chestX = W - 2 - cw;
        if (w.chestX < 1) w.chestX = 1;
    }

    w.weed.clear();
    int nweed = std::clamp(W / 8, 3, 16);
    int maxlen = std::clamp(H - 6, 3, 14);
    static const uint8_t greens[5] = {22, 28, 34, 40, 46};
    for (int i = 0; i < nweed; ++i) {
        Seaweed s;
        s.x = irand(1, std::max(1, W - 2));
        s.len = irand(3, std::max(3, maxlen));
        s.phase = frand(0.0f, 6.283f);
        s.tscale = frand(0.8f, 1.7f);
        s.col = greens[irand(0, 4)];
        w.weed.push_back(s);
    }

    w.fish.clear();
    int nfish = std::clamp((W * H) / 420, 4, 16);
    for (int i = 0; i < nfish; ++i) w.fish.push_back(makeFish(W, H));

    w.bubbles.clear();
    w.sparks.clear();
    w.chestOpen = false;
    w.chestTimer = w.time + frand(3.0f, 7.0f);
}

void spawnBubble(World& w, float x, float y, float scale) {
    if (w.bubbles.size() > 80) return;
    Bubble b;
    b.x = x;
    b.y = y;
    b.vy = frand(2.0f, 5.5f) * scale;
    b.wob = frand(0.0f, 6.283f);
    b.wamp = frand(0.2f, 1.4f);
    int r = irand(0, 9);
    if (r < 5) {
        b.ch = 'o';
        b.col = 195;
    } else if (r < 8) {
        b.ch = 'O';
        b.col = 159;
    } else {
        b.ch = '.';
        b.col = 123;
    }
    w.bubbles.push_back(b);
}

void spawnSpark(World& w, float x, float y, int burst) {
    for (int i = 0; i < burst && w.sparks.size() < 120; ++i) {
        Spark s;
        s.x = x + frand(-2.0f, 2.0f);
        s.y = y + frand(-1.0f, 1.0f);
        s.vy = frand(-3.5f, -1.2f);
        s.life = s.maxlife = frand(0.8f, 1.9f);
        int r = irand(0, 9);
        if (r < 3) {
            s.ch = '$';
            s.col = 220;
        } else if (r < 6) {
            s.ch = '*';
            s.col = 226;
        } else if (r < 8) {
            s.ch = '+';
            s.col = 229;
        } else {
            s.ch = '.';
            s.col = 195;
        }
        w.sparks.push_back(s);
    }
}

void updateWorld(World& w, float dt) {
    const int W = w.W, H = w.H;

    for (Fish& f : w.fish) {
        const Species& s = g_species[size_t(f.sp)];
        int aw = int(s.right.size());
        float dart = 0.75f + 0.45f * std::sin(w.time * 1.7f + f.dartPhase);
        f.x += f.dir * f.speed * dart * dt;
        f.y += f.vy * dt;
        f.bobPhase += f.bobSpeed * dt;

        if (frand(0.0f, 1.0f) < 0.06f * dt) f.dir = -f.dir;
        if (f.x < 1.0f) {
            f.x = 1.0f;
            f.dir = 1;
        }
        if (f.x + float(aw) > float(W - 1)) {
            f.x = float(std::max(1, W - 1 - aw));
            f.dir = -1;
        }
        float ymin = 1.0f;
        float ymax = std::max(2.0f, float(H - 4));
        if (f.y < ymin) {
            f.y = ymin;
            f.vy = std::fabs(f.vy);
        }
        if (f.y > ymax) {
            f.y = ymax;
            f.vy = -std::fabs(f.vy);
        }
        if (frand(0.0f, 1.0f) < 0.15f * dt) f.vy = frand(-0.5f, 0.5f);

        if (frand(0.0f, 1.0f) < 0.35f * dt) {
            int wy = int(std::lround(f.y + std::sin(f.bobPhase) * f.bobAmp));
            float hx = (f.dir > 0) ? f.x + float(aw - 1) : f.x;
            spawnBubble(w, hx, float(wy), 0.5f);
        }
    }

    if (frand(0.0f, 1.0f) < 5.0f * dt) spawnBubble(w, frand(1.0f, float(W - 2)), float(H - 2), 1.0f);

    for (size_t i = 0; i < w.bubbles.size();) {
        Bubble& b = w.bubbles[i];
        b.y -= b.vy * dt;
        if (b.y < 1.0f) {
            w.bubbles[i] = w.bubbles.back();
            w.bubbles.pop_back();
        } else {
            ++i;
        }
    }

    for (size_t i = 0; i < w.sparks.size();) {
        Spark& s = w.sparks[i];
        s.y += s.vy * dt;
        s.vy += 1.6f * dt;
        s.life -= dt;
        if (s.life <= 0.0f || s.y < 0.5f) {
            w.sparks[i] = w.sparks.back();
            w.sparks.pop_back();
        } else {
            ++i;
        }
    }

    if (w.chestX >= 0) {
        if (w.chestOpen) {
            if (w.time > w.chestTimer) {
                w.chestOpen = false;
                w.chestTimer = w.time + frand(6.0f, 12.0f);
            } else if (frand(0.0f, 1.0f) < 7.0f * dt) {
                spawnSpark(w, float(w.chestX + 5), float(H - 5), 1);
            }
        } else if (w.time > w.chestTimer) {
            w.chestOpen = true;
            w.chestTimer = w.time + frand(3.0f, 6.0f);
            spawnSpark(w, float(w.chestX + 5), float(H - 5), 9);
        }
    }
}

void drawSand(World& w) {
    for (int x = 0; x < w.W; ++x) {
        int h = w.sandH[size_t(x)];
        for (int k = 0; k < h; ++k) {
            int y = w.H - 1 - k;
            uint32_t v = hash32(uint32_t(x) * 0x9e3779b9u ^ uint32_t(y) * 0x85ebca6bu);
            uint32_t m = v % 100;
            char c = '.';
            if (m < 40) c = '.';
            else if (m < 65) c = ':';
            else if (m < 78) c = ',';
            else if (m < 88) c = '`';
            else if (m < 96) c = '\'';
            else c = '~';
            uint8_t f = ((v >> 9) % 5 == 0) ? 137 : 180;
            w.cv.put(x, y, c, f, SAND_BG);
        }
    }
}

void drawWeed(World& w) {
    for (const Seaweed& s : w.weed) {
        int col = std::clamp(s.x, 0, w.W - 1);
        int bottom = w.H - w.sandH[size_t(col)] - 1;
        for (int i = 0; i < s.len; ++i) {
            int y = bottom - i;
            if (y < 1) break;
            float t = w.time * s.tscale + s.phase + float(i) * 0.45f;
            float amp = 0.12f * float(i + 1);
            int off = int(std::lround(std::sin(t) * std::min(2.2f, amp)));
            char c;
            if (off > 0) c = (i % 4 == 1) ? '}' : ')';
            else if (off < 0) c = (i % 4 == 1) ? '{' : '(';
            else c = '|';
            w.cv.put(s.x + off, y, c, s.col, waterBG(y, w.H));
        }
    }
}

void drawChest(World& w) {
    if (w.chestX < 0) return;
    static const char* const closed[4] = {
        "  .-------. ",
        " /  _____  \\",
        "|  |_[]_|  |",
        " \\_________/"
    };
    static const char* const opened[4] = {
        "  ________  ",
        " /        \\ ",
        "|  *$$$$*  |",
        " \\________/ "
    };
    const char* const* art = w.chestOpen ? opened : closed;
    int top = w.H - 4;
    for (int r = 0; r < 4; ++r) {
        int y = top + r;
        if (y < 0 || y >= w.H) continue;
        for (int i = 0; i < 12; ++i) {
            char c = art[r][i];
            if (c == ' ') continue;
            uint8_t f = 130;
            if (c == '[' || c == ']') f = 178;
            else if (c == '$') f = 220;
            else if (c == '*') f = 226;
            int x = w.chestX + i;
            int sandtop = w.H - w.sandH[size_t(std::clamp(x, 0, w.W - 1))];
            uint8_t bg = (y >= sandtop) ? SAND_BG : waterBG(y, w.H);
            w.cv.put(x, y, c, f, bg);
        }
    }
}

void drawFish(World& w) {
    for (const Fish& f : w.fish) {
        const Species& s = g_species[size_t(f.sp)];
        const std::string& art = (f.dir > 0) ? s.right : s.left;
        int len = int(art.size());
        int y = int(std::lround(f.y + std::sin(f.bobPhase) * f.bobAmp));
        int x = int(std::lround(f.x));
        for (int i = 0; i < len; ++i) {
            char c = art[size_t(i)];
            uint8_t col = s.body;
            bool tail = (f.dir > 0) ? (i == 0) : (i == len - 1);
            if (c == 'o' || c == 'O' || c == '*') col = 231;
            else if (tail) col = s.fin;
            w.cv.put(x + i, y, c, col, waterBG(y, w.H));
        }
    }
}

void drawBubbles(World& w) {
    for (const Bubble& b : w.bubbles) {
        int x = int(std::lround(b.x + std::sin(b.wob + w.time * 2.4f + b.y * 0.3f) * b.wamp));
        int y = int(std::lround(b.y));
        w.cv.put(x, y, b.ch, b.col, waterBG(y, w.H));
    }
}

void drawSparks(World& w) {
    for (const Spark& s : w.sparks) {
        int x = int(std::lround(s.x));
        int y = int(std::lround(s.y));
        w.cv.put(x, y, s.ch, s.col, waterBG(y, w.H));
    }
}

void drawHint(World& w) {
    if (w.time > 8.0f || w.W < 30) return;
    const std::string hint = " ASCII AQUARIUM   [q] quit ";
    int x = w.W - int(hint.size()) - 1;
    if (x > 1) w.cv.text(x, 0, hint, 110, waterBG(0, w.H));
}

void drawWorld(World& w) {
    Canvas& cv = w.cv;
    for (int y = 0; y < w.H; ++y) {
        uint8_t bg = waterBG(y, w.H);
        for (int x = 0; x < w.W; ++x) cv.put(x, y, ' ', 7, bg);
    }
    drawSand(w);
    drawWeed(w);
    drawChest(w);
    drawFish(w);
    drawBubbles(w);
    drawSparks(w);
    drawHint(w);
}

void drawTooSmall(World& w) {
    Canvas& cv = w.cv;
    for (int y = 0; y < w.H; ++y)
        for (int x = 0; x < w.W; ++x) cv.put(x, y, ' ', 7, 17);
    const std::string l1 = "Terminal too small";
    const std::string l2 = "need at least 24x8 - press q";
    auto center = [&](int y, const std::string& s, uint8_t f) {
        int x = (w.W - int(s.size())) / 2;
        if (x < 0 || y < 0 || y >= w.H) return;
        cv.text(x, y, s, f, 17);
    };
    if (w.H >= 3) {
        center(w.H / 2 - 1, l1, 226);
        center(w.H / 2 + 1, l2, 51);
    }
}

}  // namespace

int main() {
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);
    std::atexit(restoreTerm);

    setupTerm();
    initSpecies();

    World w;
    int W = 80, H = 24;
    getSize(W, H);
    buildWorld(w, W, H);

    const auto frameDelay = std::chrono::milliseconds(25);
    auto last = Clock::now();
    auto next = last;

    while (!g_quit && !g_stop) {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (dt < 0.0f) dt = 0.0f;
        if (dt > 0.12f) dt = 0.12f;

        getSize(W, H);
        if (W != w.W || H != w.H) buildWorld(w, W, H);

        pollInput();
        w.time += dt;
        updateWorld(w, dt);

        if (w.W < 24 || w.H < 8) drawTooSmall(w);
        else drawWorld(w);
        writeAll(w.cv.render());

        next += frameDelay;
        auto sleepFor = next - Clock::now();
        if (sleepFor.count() > 0)
            std::this_thread::sleep_for(sleepFor);
        else
            next = Clock::now();
    }

    writeAll("\x1b[0m\x1b[2J");
    return 0;
}