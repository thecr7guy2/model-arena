#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <csignal>
#include <cstring>

using namespace std;
using namespace chrono;

const double PI = 3.14159265358979323846;

const string RST = "\033[0m";
const string BLD = "\033[1m";
const string WTR = "\033[44m";
const string CYN = "\033[36m";
const string GRN = "\033[32m";
const string YLW = "\033[33m";
const string RED = "\033[31m";
const string MAG = "\033[35m";
const string WHT = "\033[37m";

const vector<string> FISH_R = {"<><", "<o))><", "<*)))><", "<o))))><", "<*))))))><"};

struct Species {
    int art; string color; double mn, mx;
};

const vector<Species> SPS = {
    {0, CYN, 0.6, 1.5}, {1, GRN, 0.4, 1.0}, {2, YLW, 0.3, 0.8},
    {3, MAG, 0.2, 0.6}, {4, RED, 0.1, 0.4}, {1, WHT, 0.4, 1.2},
    {2, CYN, 0.3, 0.9}, {0, GRN, 0.5, 1.3},
};

struct Fish {
    double x, y, spd; bool right; int sp;
    double ph, bA, bF;
};

struct Bbl {
    double x, y, spd, ph, sway; char ch;
};

struct Weed {
    int x; double ph, freq, amp; int h;
};

struct Chest {
    int x, y; bool open; double tmr, opnT, clsT;
};

atomic<bool> gQ(false);
int W = 80, H = 24;
mt19937 rng(random_device{}());
uniform_real_distribution<double> rnd(0, 1);

string fishArt(int sp, bool right) {
    string s = FISH_R[SPS[sp].art];
    if (!right) reverse(s.begin(), s.end());
    return s;
}

void setupTerm() {
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void resetTerm() {
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void getSize() {
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (ws.ws_col > 0) W = ws.ws_col;
    if (ws.ws_row > 0) H = ws.ws_row;
}

int main() {
    getSize();
    setupTerm();
    cout << "\033[?25l\033[2J" << flush;

    auto cleanup = [](int) {
        resetTerm();
        cout << RST << "\033[?25h\033[2J\033[1;1H" << flush;
        exit(0);
    };
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    vector<Fish> fish;
    int nf = max(3, min(8, W / 10));
    for (int i = 0; i < nf; i++) {
        int si = i % SPS.size();
        fish.push_back({rnd(rng) * W, 1 + rnd(rng) * max(1, H - 6),
            SPS[si].mn + rnd(rng) * (SPS[si].mx - SPS[si].mn),
            rnd(rng) > 0.5, si,
            rnd(rng) * 2 * PI, 0.3 + rnd(rng) * 0.5, 0.5 + rnd(rng) * 1.0});
    }

    vector<Bbl> bbls;

    vector<Weed> weeds;
    int nw = max(3, W / 12);
    for (int i = 0; i < nw; i++) {
        weeds.push_back({1 + i * (W - 2) / max(1, nw - 1),
            rnd(rng) * 2 * PI, 0.3 + rnd(rng) * 0.4, 1.0 + rnd(rng) * 2.0,
            2 + (int)(rnd(rng) * 5)});
    }

    Chest chest = {W / 2 - 3, H - 4, false, 0, 4.0, 8.0 + rnd(rng) * 4.0};

    thread thr([]() {
        while (!gQ) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0 && (c == 'q' || c == 'Q'))
                gQ = true;
        }
    });
    thr.detach();

    double dt = 1.0 / 30.0;
    auto last = steady_clock::now();

    while (!gQ) {
        auto now = steady_clock::now();
        double elapsed = duration<double>(now - last).count();
        last = now;
        if (elapsed > 0.1) elapsed = dt;
        double t = duration<double>(steady_clock::now().time_since_epoch()).count();

        getSize();

        for (auto& f : fish) {
            f.x += (f.right ? 1 : -1) * f.spd * elapsed;
            f.y += sin(t * f.bF + f.ph) * f.bA * elapsed;
            f.y = max(1.0, min((double)H - 5, f.y));

            int len = (int)fishArt(f.sp, f.right).size();
            if (f.x < -len - 2) f.x = W + 2;
            if (f.x > W + 2) f.x = -len - 2;
            if (f.x < 0 && !f.right) f.right = true;
            if (f.x > W - len && f.right) f.right = false;
            if (rnd(rng) < 0.003) f.right = !f.right;
        }

        if (rnd(rng) < 0.4) {
            bbls.push_back({rnd(rng) * W, (double)H - 1,
                2 + rnd(rng) * 4, rnd(rng) * 2 * PI, 0.5 + rnd(rng) * 1.5,
                rnd(rng) < 0.5 ? 'o' : (rnd(rng) < 0.75 ? 'O' : '0')});
        }
        for (auto it = bbls.begin(); it != bbls.end();) {
            it->y -= it->spd * elapsed;
            it->x += sin(t * 2 + it->ph) * it->sway * elapsed;
            if (it->y < 0) it = bbls.erase(it);
            else ++it;
        }
        if (bbls.size() > 60) bbls.erase(bbls.begin(), bbls.begin() + (bbls.size() - 60));

        for (auto& w : weeds) w.ph += w.freq * elapsed;

        chest.tmr += elapsed;
        if (chest.open && chest.tmr > chest.opnT) {
            chest.open = false; chest.tmr = 0;
            chest.clsT = 4 + rnd(rng) * 6;
        } else if (!chest.open && chest.tmr > chest.clsT) {
            chest.open = true; chest.tmr = 0;
            chest.opnT = 2 + rnd(rng) * 3;
        }

        vector<vector<char>> scr(H, vector<char>(W, ' '));
        vector<vector<string>> col(H, vector<string>(W, WTR));

        double wavePh = t * 0.5;
        for (int x = 0; x < W; x++) {
            if (sin(x * 0.25 + wavePh) > 0.4) {
                scr[0][x] = '~';
                col[0][x] = CYN + WTR;
            }
        }

        for (auto& w : weeds) {
            for (int h = 0; h < w.h; h++) {
                int yy = H - 2 - h;
                if (yy < 0) continue;
                double sway = sin(w.ph + h * 0.5) * w.amp;
                int xx = w.x + (int)round(sway);
                if (xx < 0 || xx >= W) continue;
                char c = (h == w.h - 1) ? (sin(w.ph + h * 0.5) > 0 ? '/' : '\\') : '|';
                scr[yy][xx] = c;
                col[yy][xx] = GRN + WTR;
            }
        }

        {
            string r1 = chest.open ? "+~~~~~+" : "+-----+";
            string r2 = chest.open ? "| *** |" : "|#####|";
            for (size_t i = 0; i < r1.size(); i++) {
                int cx = chest.x + (int)i;
                if (cx >= 0 && cx < W) {
                    scr[chest.y][cx] = r1[i];
                    col[chest.y][cx] = YLW + WTR;
                }
            }
            for (size_t i = 0; i < r2.size(); i++) {
                int cx = chest.x + (int)i;
                if (cx >= 0 && cx < W && chest.y + 1 < H) {
                    scr[chest.y + 1][cx] = r2[i];
                    col[chest.y + 1][cx] = YLW + WTR;
                }
            }

            if (chest.open) {
                int starX = chest.x + 3;
                int starY = chest.y + 1;
                if (starX >= 0 && starX < W && starY >= 0 && starY < H) {
                    char stars[] = {'*', '$', '%', '@', '&'};
                    int idx = (int)(t * 3) % 5;
                    scr[starY][starX] = stars[idx];
                }
            }
        }

        for (auto& b : bbls) {
            int bx = (int)round(b.x);
            int by = (int)round(b.y);
            if (bx >= 0 && bx < W && by >= 0 && by < H && scr[by][bx] == ' ') {
                scr[by][bx] = b.ch;
                col[by][bx] = CYN + WTR;
            }
        }

        for (auto& f : fish) {
            string art = fishArt(f.sp, f.right);
            string fc = SPS[f.sp].color + WTR;
            int fx = (int)round(f.x);
            int fy = (int)round(f.y);
            for (size_t i = 0; i < art.size(); i++) {
                int xx = fx + (int)i;
                if (xx >= 0 && xx < W && fy >= 0 && fy < H) {
                    scr[fy][xx] = art[i];
                    col[fy][xx] = fc;
                }
            }
        }

        stringstream out;
        out << "\033[1;1H";
        string lc;
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                if (col[y][x] != lc) {
                    out << col[y][x];
                    lc = col[y][x];
                }
                out << scr[y][x];
            }
            out << RST << "\n";
            lc = "";
        }
        cout << out.str() << flush;

        this_thread::sleep_until(now + milliseconds((int)(dt * 1000)));
    }

    resetTerm();
    cout << RST << "\033[?25h\033[2J\033[1;1H" << flush;
    return 0;
}
