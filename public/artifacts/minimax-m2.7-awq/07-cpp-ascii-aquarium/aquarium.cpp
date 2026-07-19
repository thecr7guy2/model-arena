#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

struct Point { int x, y; };

enum class FishSpecies { SMALL, MEDIUM, LARGE, JELLYFISH };
enum class Direction { LEFT, RIGHT, UP, DOWN };

struct Fish {
    Point pos;
    Direction dir;
    FishSpecies species;
    int speed;
    int color;
    int frame;
    int move_counter;
    bool active;
};

struct Bubble {
    Point pos;
    int color;
    int speed;
    bool active;
};

struct Seaweed {
    Point pos;
    int height;
    int phase;
    bool active;
};

struct TreasureChest {
    Point pos;
    bool open;
    int timer;
    int phase;
    bool active;
};

struct Aquarium {
    std::vector<Fish> fishes;
    std::vector<Bubble> bubbles;
    std::vector<Seaweed> seaweeds;
    std::vector<TreasureChest> chests;
    int width, height;
    std::atomic<bool> running;
    std::atomic<char> input_char;
};

void getTerminalSize(int& width, int& height) {
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        width = ws.ws_col > 0 ? ws.ws_col : 80;
        height = ws.ws_row > 0 ? ws.ws_row : 24;
    } else {
        width = 80;
        height = 24;
    }
}

void hideCursor() { std::cout << "\033[?25l"; }
void showCursor() { std::cout << "\033[?25h"; }
void clearScreen() { std::cout << "\033[2J"; }
void moveTo(int y, int x) { std::cout << "\033[" << y << ";" << x << "H"; }

std::string getFishArt(FishSpecies species, Direction dir, int frame) {
    bool flip = (dir == Direction::LEFT);
    if (species == FishSpecies::SMALL) {
        if (flip) {
            return frame % 2 == 0 ? "<>" : "<>";
        } else {
            return frame % 2 == 0 ? "<>" : "<>";
        }
    } else if (species == FishSpecies::MEDIUM) {
        if (flip) {
            return frame % 2 == 0 ? "><)))=<" : "><)))><";
        } else {
            return frame % 2 == 0 ? "<=(((><" : "<=)))><";
        }
    } else if (species == FishSpecies::LARGE) {
        if (flip) {
            return frame % 2 == 0 ? "><(((>>" : "><)))>>";
        } else {
            return frame % 2 == 0 ? "<<<)))><" : "<<<)))><";
        }
    } else {
        if (flip) {
            return frame % 2 == 0 ? " .-. " : "(   )";
        } else {
            return frame % 2 == 0 ? " .-. " : "(   )";
        }
    }
}

std::string getJellyfishBody(int frame) {
    return frame % 2 == 0 ? "\\___/" : "\\---/";
}

std::string getJellyfishTentacles() {
    return " | |  | |";
}

void drawFish(const Fish& fish, int term_height) {
    if (!fish.active) return;
    std::string art = getFishArt(fish.species, fish.dir, fish.frame);
    if (fish.species == FishSpecies::JELLYFISH) {
        std::cout << "\033[34m";
        int y = fish.pos.y - 1;
        if (y >= 1 && y < term_height - 2) {
            moveTo(y, fish.pos.x);
            std::cout << getJellyfishBody(fish.frame);
            moveTo(y + 1, fish.pos.x);
            std::cout << getJellyfishTentacles();
        }
    } else {
        std::cout << "\033[" << fish.color << "m";
        int y = fish.pos.y;
        if (y >= 1 && y < term_height - 1) {
            moveTo(y, fish.pos.x);
            std::cout << art;
        }
    }
    std::cout << "\033[0m";
}

void drawBubble(const Bubble& bubble, int term_height) {
    if (!bubble.active) return;
    int y = bubble.pos.y;
    if (y >= 1 && y < term_height - 1) {
        moveTo(y, bubble.pos.x);
        std::cout << "\033[36m" << "o" << "\033[0m";
    }
}

char getSeaweedChar(int phase) {
    switch(phase % 4) {
        case 0: return '\\';
        case 1: return '/';
        case 2: return '\\';
        default: return '/';
    }
}

void drawSeaweed(const Seaweed& seaweed, int term_height) {
    if (!seaweed.active) return;
    std::cout << "\033[32m";
    for (int i = 0; i < seaweed.height && seaweed.pos.y + i < term_height - 1; ++i) {
        int y = seaweed.pos.y + i;
        if (y >= 1) {
            moveTo(y, seaweed.pos.x);
            int phase = (seaweed.phase + i) % 4;
            std::cout << getSeaweedChar(phase);
        }
    }
    std::cout << "\033[0m";
}

void drawChest(const TreasureChest& chest, int term_height) {
    if (!chest.active) return;
    int y = chest.pos.y;
    if (y >= 1 && y < term_height - 1) {
        std::cout << "\033[33m";
        if (chest.open) {
            if (chest.phase % 2 == 0) {
                moveTo(y, chest.pos.x);
                std::cout << "  ___  ";
                moveTo(y + 1, chest.pos.x);
                std::cout << " /$$$\\";
                moveTo(y + 2, chest.pos.x);
                std::cout << " \\___/ ";
            } else {
                moveTo(y, chest.pos.x);
                std::cout << "  ---  ";
                moveTo(y + 1, chest.pos.x);
                std::cout << " /$$$\\";
                moveTo(y + 2, chest.pos.x);
                std::cout << " \\---/ ";
            }
        } else {
            moveTo(y, chest.pos.x);
            std::cout << "  ___  ";
            moveTo(y + 1, chest.pos.x);
            std::cout << " |###| ";
            moveTo(y + 2, chest.pos.x);
            std::cout << " |___| ";
        }
        std::cout << "\033[0m";
    }
}

void drawFrame(Aquarium& aq) {
    clearScreen();
    std::string border_top(aq.width - 2, '~');
    std::cout << "\033[36m";
    moveTo(0, 1);
    std::cout << border_top;
    moveTo(aq.height - 1, 1);
    std::cout << border_top;
    for (int i = 1; i < aq.height - 1; ++i) {
        moveTo(i, 0);
        std::cout << ":";
        moveTo(i, aq.width - 1);
        std::cout << ":";
    }
    std::cout << "\033[0m";

    for (auto& sw : aq.seaweeds) drawSeaweed(sw, aq.height);
    for (auto& b : aq.bubbles) drawBubble(b, aq.height);
    for (auto& c : aq.chests) drawChest(c, aq.height);
    for (auto& f : aq.fishes) drawFish(f, aq.height);

    moveTo(aq.height - 1, aq.width / 2 - 10);
    std::cout << "Press 'q' to quit" << std::flush;
}

void initAquarium(Aquarium& aq) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> speedDist(1, 3);
    std::uniform_int_distribution<> colorDist(31, 37);
    std::uniform_int_distribution<> xDist(2, aq.width - 10);
    std::uniform_int_distribution<> yDist(3, aq.height - 4);

    for (int i = 0; i < 12; ++i) {
        Fish f;
        f.pos.x = xDist(gen);
        f.pos.y = yDist(gen);
        f.dir = gen() % 2 == 0 ? Direction::LEFT : Direction::RIGHT;
        f.speed = speedDist(gen);
        f.color = colorDist(gen);
        f.frame = 0;
        f.move_counter = 0;
        f.active = true;
        int speciesRoll = gen() % 10;
        if (speciesRoll < 4) f.species = FishSpecies::SMALL;
        else if (speciesRoll < 7) f.species = FishSpecies::MEDIUM;
        else if (speciesRoll < 9) f.species = FishSpecies::LARGE;
        else f.species = FishSpecies::JELLYFISH;
        aq.fishes.push_back(f);
    }

    for (int i = 0; i < 20; ++i) {
        Bubble b;
        b.pos.x = xDist(gen);
        b.pos.y = yDist(gen);
        b.color = 36;
        b.speed = 1;
        b.active = true;
        aq.bubbles.push_back(b);
    }

    for (int i = 0; i < aq.width / 8; ++i) {
        Seaweed sw;
        sw.pos.x = 3 + i * 7;
        sw.pos.y = aq.height - 3;
        sw.height = 3 + gen() % 4;
        sw.phase = gen() % 4;
        sw.active = true;
        aq.seaweeds.push_back(sw);
    }

    for (int i = 0; i < 2; ++i) {
        TreasureChest chest;
        chest.pos.x = 10 + i * (aq.width / 2);
        chest.pos.y = aq.height - 3;
        chest.open = false;
        chest.timer = 0;
        chest.phase = 0;
        chest.active = true;
        aq.chests.push_back(chest);
    }
}

void updateAquarium(Aquarium& aq) {
    std::random_device rd;
    std::mt19937 gen(rd());
    while (aq.running) {
        std::uniform_int_distribution<> dirDist(0, 3);
        std::uniform_int_distribution<> xDist(2, aq.width - 10);
        std::uniform_int_distribution<> yDist(3, aq.height - 4);

        for (auto& f : aq.fishes) {
            if (!f.active) continue;
            ++f.move_counter;
            if (f.move_counter >= f.speed) {
                f.move_counter = 0;
                ++f.frame;

                if (f.dir == Direction::LEFT) {
                    --f.pos.x;
                    if (f.pos.x <= 2) {
                        f.dir = Direction::RIGHT;
                        f.pos.x = 2;
                    }
                } else {
                    ++f.pos.x;
                    if (f.pos.x >= aq.width - 8) {
                        f.dir = Direction::LEFT;
                        f.pos.x = aq.width - 8;
                    }
                }

                if (gen() % 20 == 0) {
                    int newDir = dirDist(gen);
                    if (newDir == 0) f.dir = Direction::LEFT;
                    else if (newDir == 1) f.dir = Direction::RIGHT;
                }
            }
        }

        for (auto& b : aq.bubbles) {
            if (!b.active) continue;
            --b.pos.y;
            if (b.pos.y <= 2) {
                b.pos.y = aq.height - 3;
                b.pos.x = xDist(gen);
            }
        }

        for (auto& sw : aq.seaweeds) {
            if (!sw.active) continue;
            ++sw.phase;
            if (sw.phase > 3) sw.phase = 0;
        }

        for (auto& chest : aq.chests) {
            if (!chest.active) continue;
            ++chest.timer;
            ++chest.phase;
            if (chest.timer > 100) {
                chest.open = !chest.open;
                chest.timer = 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void inputReader(Aquarium& aq) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (aq.running) {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'q' || c == 'Q') {
                aq.running = false;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main() {
    int term_width, term_height;
    getTerminalSize(term_width, term_height);

    Aquarium aq;
    aq.width = term_width;
    aq.height = term_height;
    aq.running = true;
    aq.input_char = 0;

    initAquarium(aq);

    hideCursor();
    clearScreen();

    std::thread update_thread([&aq]() { updateAquarium(aq); });
    std::thread input_thread([&aq]() { inputReader(aq); });

    auto last_render = std::chrono::steady_clock::now();
    const auto frame_interval = std::chrono::milliseconds(100);

    while (aq.running) {
        auto now = std::chrono::steady_clock::now();
        if (now - last_render >= frame_interval) {
            drawFrame(aq);
            last_render = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    input_thread.join();
    update_thread.join();

    clearScreen();
    showCursor();
    std::cout << "\033[0m";
    std::cout.flush();

    return 0;
}