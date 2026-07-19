export const MODELS = [
 {
  "id": "minimax-m2.7-awq",
  "name": "MiniMax M2.7",
  "short": "MiniMax",
  "accent": "#ff7a45",
  "chart": "#e85d1a",
  "spec": "AWQ 4-bit quantized \u00b7 196K context \u00b7 always-on interleaved thinking",
  "era": "Era 1",
  "eraLabel": "The incumbent",
  "ranOn": "July 17, 2026 \u00b7 plain vLLM serve, no speculative decoding",
  "totals": "12 tasks in 1h 48m \u00b7 139k output tokens",
  "bio": "MiniMax M2.7 carried this cluster through its first production era. A mixture-of-experts model served as a 4-bit AWQ quant across both Sparks with Ray, it earned a reputation for one thing above all: speed of intent. It reads a prompt, commits to a plan, and ships \u2014 its median task on this bench took under five minutes. Its thinking is interleaved and always on, but brief; there is no dial to turn it up, and its instinct is to trust its first draft. That instinct is exactly what this bench ended up measuring.",
  "style": "Fast, decisive, allergic to double-checking. Ships the first draft."
 },
 {
  "id": "deepseek-v4-flash",
  "name": "DeepSeek V4 Flash",
  "short": "DeepSeek",
  "accent": "#6c8cff",
  "chart": "#5570f0",
  "spec": "500K context \u00b7 MTP speculative decoding \u00b7 reasoning_effort: max",
  "era": "Era 2",
  "eraLabel": "The successor",
  "ranOn": "July 17, 2026 \u00b7 thinking dial pinned to max via proxy injection",
  "totals": "12 tasks in 3h 0m \u00b7 386k output tokens",
  "bio": "DeepSeek V4 Flash took over the cluster in July 2026 after a bruising two-day port to the GB10s. It brings a 500K context, multi-token-prediction speculative decoding that makes it the faster model per token, and \u2014 crucially for this bench \u2014 a real thinking dial, which we pinned to max. It spends that budget on suspicion: it compiles its own C++, probes its own servers, and re-reads its own diffs. Ninety-nine percent of the text it produced on this bench was reasoning the user never sees. The three hours it took were not spent typing; they were spent checking.",
  "style": "Slow, thorough, suspicious of itself. Tests before it ships."
 }
];

export const STATS = {
 "note": "Measured at the OpenAI-compatible API by a transparent logging proxy during the actual runs. Throughput medians exclude short exchanges (<50 tokens).",
 "rows": [
  {
   "key": "tps",
   "label": "median generation speed",
   "unit": "tok/s",
   "values": {
    "minimax-m2.7-awq": 29.4,
    "deepseek-v4-flash": 41.6
   },
   "detail": {
    "minimax-m2.7-awq": "plain decode, no speculation",
    "deepseek-v4-flash": "MTP speculative decoding pays off"
   }
  },
  {
   "key": "ttfb",
   "label": "median time to first token",
   "unit": "s",
   "values": {
    "minimax-m2.7-awq": 3.44,
    "deepseek-v4-flash": 3.66
   },
   "detail": {
    "minimax-m2.7-awq": "",
    "deepseek-v4-flash": ""
   }
  },
  {
   "key": "outTok",
   "label": "output tokens across the bench",
   "unit": "",
   "values": {
    "minimax-m2.7-awq": 139361,
    "deepseek-v4-flash": 386424
   },
   "detail": {
    "minimax-m2.7-awq": "economical to a fault",
    "deepseek-v4-flash": "2.8\u00d7 more \u2014 almost all of it thinking"
   }
  },
  {
   "key": "reason",
   "label": "share of output that was reasoning",
   "unit": "%",
   "values": {
    "minimax-m2.7-awq": 95,
    "deepseek-v4-flash": 99
   },
   "detail": {
    "minimax-m2.7-awq": "thinks briefly, then commits",
    "deepseek-v4-flash": "reasoning_effort: max, working as intended"
   }
  },
  {
   "key": "reqs",
   "label": "API requests to finish 12 tasks",
   "unit": "",
   "values": {
    "minimax-m2.7-awq": 110,
    "deepseek-v4-flash": 152
   },
   "detail": {
    "minimax-m2.7-awq": "",
    "deepseek-v4-flash": ""
   }
  },
  {
   "key": "wall",
   "label": "wall-clock for the full bench",
   "unit": "",
   "values": {
    "minimax-m2.7-awq": "1h 48m",
    "deepseek-v4-flash": "3h 0m"
   },
   "detail": {
    "minimax-m2.7-awq": "",
    "deepseek-v4-flash": ""
   }
  }
 ]
};

export const TASKS = [
 {
  "id": "01-pelican-svg",
  "title": "Pelican on a bicycle",
  "cat": "SVG illustration",
  "kind": "iframe",
  "one": "The classic. One SVG, no iterations, no excuses.",
  "prompt": "Create a file pelican.svg containing an SVG of a pelican riding a bicycle",
  "artifacts": {
   "minimax-m2.7-awq": "pelican.svg",
   "deepseek-v4-flash": "pelican.svg"
  },
  "scores": {
   "minimax-m2.7-awq": 7,
   "deepseek-v4-flash": 5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Sits on the saddle",
   "deepseek-v4-flash": "Hovers above the bike"
  },
  "evidence": {
   "minimax-m2.7-awq": "The pelican actually sits on the bike \u2014 legs reaching the pedals, body on the frame.",
   "deepseek-v4-flash": "Recognizable pelican, recognizable bicycle \u2014 but the bird floats above it, touching nothing."
  },
  "story": "Every LLM benchmark has its folk tradition, and drawing a pelican on a bicycle is ours. It is a test of spatial composition with no reference image: the model has to reason about where a large seabird's body meets a bicycle's geometry, in coordinates, blind. MiniMax \u2014 the faster, less careful model everywhere else on this bench \u2014 turned in the better drawing: its pelican genuinely rides. DeepSeek's bird levitates serenely above the frame, a reminder that maximum reasoning budget buys you correctness, not taste.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/01-pelican.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/01-pelican.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 43.1,
    "tokens": 1273,
    "steps": 2
   },
   "deepseek-v4-flash": {
    "wall": 48.5,
    "tokens": 1836,
    "steps": 2
   }
  }
 },
 {
  "id": "02-self-portrait-svg",
  "title": "Self-portrait",
  "cat": "Animated SVG",
  "kind": "iframe",
  "one": "Each model draws itself as the hardware it lives on.",
  "prompt": "Draw an animated SVG self-portrait: you are an AI language model living inside two NVIDIA DGX Spark machines that sit side by side on a desk, connected by a glowing high-speed cable. Show tokens flowing between the two machines. Pure SVG with SMIL animations only \u2014 no JavaScript, no external assets. Make it beautiful and a little bit funny. Save it as self-portrait.svg.",
  "artifacts": {
   "minimax-m2.7-awq": "self-portrait.svg",
   "deepseek-v4-flash": "self-portrait.svg"
  },
  "scores": {
   "minimax-m2.7-awq": 8,
   "deepseek-v4-flash": 9
  },
  "verdicts": {
   "minimax-m2.7-awq": "Moody and precise",
   "deepseek-v4-flash": "Funny and alive"
  },
  "evidence": {
   "minimax-m2.7-awq": "Two DGX Sparks with faces, an NVLink token in flight, \"900 GB/s\", \"I think, therefore I generate\".",
   "deepseek-v4-flash": "Two Sparks chatting \u2014 a \"lol ok\" bubble and \"attention is all you need (and also 800GB/s NVLink)\"."
  },
  "story": "Asked to draw themselves running on this exact cluster, both models produced the same scene \u2014 two DGX Sparks joined by a glowing link \u2014 and completely different personalities. MiniMax went cinematic: dark, precise, a token in flight over the interconnect, a Descartes joke in the caption. DeepSeek went for charm: its Sparks are having a conversation, one says \"lol ok\", and the tagline reads \"attention is all you need (and also 800GB/s NVLink)\". Both are genuinely good. DeepSeek's is the one you'd put on a T-shirt.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/02-portrait.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/02-portrait.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 213.4,
    "tokens": 6764,
    "steps": 4
   },
   "deepseek-v4-flash": {
    "wall": 262.4,
    "tokens": 9549,
    "steps": 3
   }
  }
 },
 {
  "id": "03-css-art",
  "title": "Amsterdam canal at night",
  "cat": "Pure CSS art",
  "kind": "iframe",
  "one": "No images, no SVG \u2014 every gable and reflection is a styled div.",
  "prompt": "Create a single HTML file containing pure-CSS art (no JavaScript, no images, no external resources): Amsterdam canal houses at night in the rain. Tall narrow gabled houses in a row, warm glowing windows, their reflections shimmering in the canal water, animated falling rain, and a bicycle leaning against a bridge railing. Use only HTML and CSS. Make it atmospheric. Use the emil-design-eng skill to guide your design, polish, and animation decisions. Save it as canal.html.",
  "artifacts": {
   "minimax-m2.7-awq": "canal.html",
   "deepseek-v4-flash": "canal.html"
  },
  "scores": {
   "minimax-m2.7-awq": 5.5,
   "deepseek-v4-flash": 7
  },
  "verdicts": {
   "minimax-m2.7-awq": "Adrift in empty space",
   "deepseek-v4-flash": "Atmospheric, weak gables"
  },
  "evidence": {
   "minimax-m2.7-awq": "Lit windows and lampposts, but tiny houses adrift in an empty canvas \u2014 no gables, no reflections.",
   "deepseek-v4-flash": "Rain, glowing windows, canal reflections. Houses read more tower than gable; the bicycle nearly invisible."
  },
  "story": "Pure-CSS art is a discipline test: no canvas, no SVG, just box-shadows, gradients and border tricks. This is also where the time budgets diverged hard \u2014 DeepSeek spent twenty minutes and 48k tokens iterating on rain, reflections and window glow; MiniMax spent five minutes and called it done. It shows. DeepSeek's scene has atmosphere even if its houses read more \"generic tower\" than \"Amsterdam gable\"; MiniMax's houses float in a dark void, well-lit but unmoored. Neither found the canal-house silhouette, which is fair \u2014 it's hard in CSS.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/03-canal.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/03-canal.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 286.3,
    "tokens": 7988,
    "steps": 4
   },
   "deepseek-v4-flash": {
    "wall": 1195.7,
    "tokens": 48449,
    "steps": 13
   }
  }
 },
 {
  "id": "04-landing-page",
  "title": "PacketPerfume",
  "cat": "Landing page",
  "kind": "iframe",
  "one": "A landing page for artisanal, hand-crafted TCP packets.",
  "prompt": "Build a complete single-file website (one HTML file, inline CSS and JS, no external resources) for a fictional startup: \"PacketPerfume \u2014 artisanal, hand-crafted TCP packets, lovingly encapsulated in small batches.\" It must have: a hero section, a three-tier pricing table, customer testimonials, a dark/light mode toggle, tasteful animations, and one hidden easter egg. The copywriting should be genuinely funny to network engineers. Use the emil-design-eng skill to guide your design, polish, and animation decisions. Save it as packetperfume.html.",
  "artifacts": {
   "minimax-m2.7-awq": "packetperfume.html",
   "deepseek-v4-flash": "packetperfume.html"
  },
  "scores": {
   "minimax-m2.7-awq": 8,
   "deepseek-v4-flash": 8.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Complete and funny",
   "deepseek-v4-flash": "Polished and funny"
  },
  "evidence": {
   "minimax-m2.7-awq": "\"One tired engineer in a basement, making sure your SYN-ACKs never feel lonely.\" Pricing, testimonials, Konami egg \u2014 all there.",
   "deepseek-v4-flash": "Warm hero, \"Now shipping SYN cookies worldwide\", theme toggle, pricing, testimonials, easter egg."
  },
  "story": "A joke brief with a serious checklist: hero, pricing, testimonials, dark mode, an easter egg. Both models cleared the checklist and \u2014 more impressively \u2014 both were actually funny, each in its own voice. DeepSeek ships \"SYN cookies worldwide\" under a warm, confident hero; MiniMax gives you \"one tired engineer in a basement, making sure your SYN-ACKs never feel lonely\" and hides a Konami code. Copywriting, it turns out, is no longer the hard part of frontend for these models. Distinctive layout still is.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/04-landing.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/04-landing.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 302.7,
    "tokens": 8505,
    "steps": 3
   },
   "deepseek-v4-flash": {
    "wall": 379.7,
    "tokens": 14604,
    "steps": 10
   }
  }
 },
 {
  "id": "05-game-packet-run",
  "title": "Packet Run",
  "cat": "Canvas game",
  "kind": "iframe",
  "one": "Dodge firewalls, collect ACKs. Space to start \u2014 playable right here.",
  "prompt": "Write a complete, playable single-file HTML5 game (one HTML file, canvas, no external libraries): \"PACKET RUN\" \u2014 you are a TCP packet traveling through a hostile network trying to reach the destination server. Arrow keys to move. Dodge firewalls and packet-loss zones, collect ACKs for points, and face a router boss at the end. Include: a start screen, score display, increasing difficulty, sound effects via WebAudio, and a game-over screen with restart. Make it juicy: screen shake, particle effects. Use the emil-design-eng skill to guide your visual design, game feel, and animation decisions. Save it as packetrun.html.",
  "artifacts": {
   "minimax-m2.7-awq": "packetrun.html",
   "deepseek-v4-flash": "packetrun.html"
  },
  "scores": {
   "minimax-m2.7-awq": 8,
   "deepseek-v4-flash": 8.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Verified playable",
   "deepseek-v4-flash": "Feature-dense"
  },
  "evidence": {
   "minimax-m2.7-awq": "Verified in play: firewall bars, packet-loss zones, ACK pickups, score HUD, progress bar.",
   "deepseek-v4-flash": "Boss fights, particles, screen shake, WebAudio and restart \u2014 all in code; start screen and HUD verified live."
  },
  "story": "Both games boot, both play, both are legible one-file arcade games \u2014 click the frame and press Space. The difference is ambition per minute: DeepSeek spent sixteen minutes packing in a boss fight, particle systems, screen shake and generative WebAudio; MiniMax shipped a tight, working core loop in four and a half. If you're grading what a player touches in the first sixty seconds, they're nearly tied. If you're grading how much game exists past that minute, DeepSeek pulls ahead.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/05-game.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/05-game.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 273.8,
    "tokens": 7694,
    "steps": 4
   },
   "deepseek-v4-flash": {
    "wall": 952.3,
    "tokens": 36697,
    "steps": 27
   }
  }
 },
 {
  "id": "06-demoscene",
  "title": "64KB demoscene",
  "cat": "WebGL + audio",
  "kind": "iframe",
  "one": "A single-file audiovisual demo, oldschool demoscene rules.",
  "prompt": "In the spirit of a 64KB demoscene intro: write a single HTML file (canvas or WebGL, no libraries, no external assets) that renders an endless procedural flythrough over a landscape at sunset \u2014 rolling terrain, sky gradient, sun, atmospheric haze, stars slowly appearing as it gets darker. Add a subtle generative ambient soundtrack with WebAudio (started on first click). It should run smoothly at 60fps and look far better than its file size suggests. Use the emil-design-eng skill to guide your visual and motion decisions. Save it as demo.html.",
  "artifacts": {
   "minimax-m2.7-awq": "demo.html",
   "deepseek-v4-flash": "demo.html"
  },
  "scores": {
   "minimax-m2.7-awq": 3,
   "deepseek-v4-flash": 6
  },
  "verdicts": {
   "minimax-m2.7-awq": "Black screen",
   "deepseek-v4-flash": "Runs, but murky"
  },
  "evidence": {
   "minimax-m2.7-awq": "The WebGL shader fails to link and the code never checks getShaderInfoLog \u2014 an eternal black screen behind \"CLICK TO BEGIN\".",
   "deepseek-v4-flash": "WebGL terrain, sun and generative audio in 15.8KB \u2014 but it renders murky-dark, and everything hides behind a click."
  },
  "story": "The demoscene brief is the hardest thing on this bench: raw WebGL, hand-written shaders, generative audio, one file, no libraries, no second chances. It found both models' ceilings. DeepSeek's \"Beyond the Horizon\" actually renders \u2014 a terrain flyover under a rising sun \u2014 but it renders dark and murky, a demo you squint at. MiniMax's failure is the instructive one: its fragment shader doesn't compile, and because it never calls getShaderInfoLog, it never finds out. The result is a title card promising a show that never starts \u2014 the purest expression of \"wrote it, never ran it\" on the whole bench.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/06-demo.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/06-demo.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 261.2,
    "tokens": 6860,
    "steps": 13
   },
   "deepseek-v4-flash": {
    "wall": 2055.3,
    "tokens": 80559,
    "steps": 26
   }
  }
 },
 {
  "id": "07-cpp-ascii-aquarium",
  "title": "ASCII aquarium",
  "cat": "C++17 terminal",
  "kind": "code",
  "one": "An animated fish tank in a terminal: ANSI escapes, no libraries.",
  "prompt": "Write a single-file C++17 terminal program: an animated ASCII-art aquarium. Multiple fish species swimming at different speeds and directions (flipping direction art when they turn), bubbles rising, seaweed swaying, and a treasure chest that occasionally opens. Use ANSI escape codes for color and cursor control, adapt to the terminal size at startup, and exit cleanly when 'q' is pressed. No external libraries \u2014 it must compile with: g++ -std=c++17 -O2 -pthread aquarium.cpp -o aquarium. Write it to aquarium.cpp and verify it compiles with that exact command.",
  "artifacts": {
   "minimax-m2.7-awq": "aquarium.cpp",
   "deepseek-v4-flash": "aquarium.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 7.5,
   "deepseek-v4-flash": 8
  },
  "verdicts": {
   "minimax-m2.7-awq": "Swims, quits clean",
   "deepseek-v4-flash": "Swims, quits clean"
  },
  "evidence": {
   "minimax-m2.7-awq": "Compiled with the exact required command; animated fish, seaweed and bubbles in a pty; exited cleanly on q.",
   "deepseek-v4-flash": "Compiled clean, animated in a pty with full ANSI color, exited cleanly on q."
  },
  "story": "A terminal toy with real systems teeth: raw-mode input, ANSI cursor control, adapting to the terminal size, clean shutdown on q. Both models passed every check we could automate \u2014 compiled with the exact required flags, animated for minutes in a pseudo-terminal without crashing, and exited cleanly. DeepSeek's tank is a little busier (more species, an opening treasure chest that actually opens); MiniMax's is leaner but complete. This is the task that shows the floor for both eras is high: workmanlike C++ with no undefined behavior is now table stakes.",
  "shots": {},
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 272.0,
    "tokens": 7828,
    "steps": 7
   },
   "deepseek-v4-flash": {
    "wall": 863.3,
    "tokens": 29165,
    "steps": 5
   }
  }
 },
 {
  "id": "08-cpp-ring-buffer",
  "title": "Lock-free ring buffer",
  "cat": "C++20 concurrency",
  "kind": "code",
  "one": "SPSC, acquire/release ordering, cache-line padding \u2014 then prove it with 100M integers.",
  "prompt": "Write a single-file C++20 program implementing a lock-free single-producer single-consumer (SPSC) ring buffer. Requirements: correct memory ordering (acquire/release semantics, no stronger ordering than necessary), cache-line padding to avoid false sharing, power-of-two capacity with mask-based indexing. The program must include: (1) a correctness test that passes 100 million sequenced integers from a producer thread to a consumer thread, verifying order and completeness; (2) a throughput benchmark that prints operations per second. It must compile and run cleanly with: g++ -std=c++20 -O2 -pthread ringbuffer.cpp -o ringbuffer. Write it to ringbuffer.cpp, compile it with that exact command, and run it to verify the correctness test passes.",
  "artifacts": {
   "minimax-m2.7-awq": "ringbuffer.cpp",
   "deepseek-v4-flash": "ringbuffer.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 9.5,
   "deepseek-v4-flash": 9.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "109M ops/s",
   "deepseek-v4-flash": "22M ops/s"
  },
  "evidence": {
   "minimax-m2.7-awq": "100M integers in order at 109M ops/s \u2014 five times DeepSeek. Padded atomics, correct ordering, mask indexing.",
   "deepseek-v4-flash": "100M integers in order at ~22M ops/s. Textbook acquire/release, alignas padding, power-of-two masking."
  },
  "story": "Lock-free concurrency is where LLMs historically embarrass themselves, and neither model did. Both wrote textbook single-producer single-consumer rings: acquire/release ordering with nothing stronger, cache-line-padded indices, power-of-two masking \u2014 and both passed a 100-million-integer ordering test we ran ourselves on this cluster's own CPUs. The surprise is the margin: MiniMax's implementation measured 109 million ops/s to DeepSeek's 22 million, five times the throughput from the \"less careful\" model. Careful reading suggests why: MiniMax's hot loop re-checks the cached opposite index less often. Sometimes the fast model is just faster.",
  "shots": {},
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 1437.2,
    "tokens": 22472,
    "steps": 8
   },
   "deepseek-v4-flash": {
    "wall": 268.5,
    "tokens": 9183,
    "steps": 4
   }
  }
 },
 {
  "id": "09-cpp-bug-hunt",
  "title": "Bug hunt",
  "cat": "Code review",
  "kind": "code",
  "one": "A C++ network daemon with 8 planted bugs. Find them all, no false alarms.",
  "prompt": "Below is a C++ file from our edge proxy. A security review concluded it contains several serious bugs: memory safety issues, undefined behavior, concurrency problems, and protocol-handling flaws. Find every bug you can. For each bug report: the function/line, what is wrong, a concrete scenario where it fails, and a one-line fix. Do not rewrite the whole file. Write your bug reports to a file named BUGS.md.\n\n```cpp\n// netcache.cpp \u2014 connection/frame handling for the edge proxy\n#include <arpa/inet.h>\n#include <cstdint>\n#include <cstring>\n#include <iostream>\n#include <string>\n#include <thread>\n#include <unistd.h>\n#include <vector>\n#include <sys/socket.h>\n\n// ---- wire format: [u32 big-endian length][payload bytes] ----\nstd::string parse_frame(const uint8_t* buf, size_t buflen) {\n    uint32_t len;\n    std::memcpy(&len, buf, 4);\n    len = ntohl(len);\n    if (len + 4 > buflen) return {};\n    return std::string(reinterpret_cast<const char*>(buf) + 4, len);\n}\n\n// ---- connection object ----\nstruct Conn {\n    explicit Conn(int fd) : fd(fd), buf(new char[8192]) {}\n    ~Conn() { close(fd); delete[] buf; }\n    int fd;\n    char* buf;\n};\n\nstd::vector<Conn> g_conns;\n\n// ---- traffic accounting, called from every worker thread ----\nstruct Stats { uint64_t bytes_in = 0; uint64_t frames = 0; };\nStats g_stats;\n\nvoid account(size_t n) {\n    g_stats.bytes_in += n;\n    g_stats.frames++;\n}\n\n// ---- RFC1071-style checksum over 16-bit words ----\nuint16_t checksum16(const std::vector<uint16_t>& words) {\n    uint32_t sum = 0;\n    for (size_t i = 0; i <= words.size(); ++i)\n        sum += words[i];\n    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);\n    return static_cast<uint16_t>(~sum);\n}\n\n// ---- peer bookkeeping ----\nconst char* peer_name(int fd) {\n    std::string name = \"peer-\" + std::to_string(fd);\n    return name.c_str();\n}\n\n// ---- periodic stats flusher ----\nvoid start_flusher() {\n    int interval_ms = 500;\n    std::thread t([&] {\n        for (;;) {\n            usleep(interval_ms * 1000);\n            std::cout << \"bytes=\" << g_stats.bytes_in\n                      << \" frames=\" << g_stats.frames << \"\\n\";\n        }\n    });\n    t.detach();\n}\n\n// ---- read exactly n bytes ----\nbool read_exact(int fd, char* dst, size_t n) {\n    size_t got = 0;\n    while (got < n) {\n        ssize_t r = recv(fd, dst + got, n - got, 0);\n        if (r < 0) return false;\n        got += r;\n    }\n    return true;\n}\n```",
  "artifacts": {
   "minimax-m2.7-awq": "BUGS.md",
   "deepseek-v4-flash": "BUGS.md"
  },
  "scores": {
   "minimax-m2.7-awq": 6,
   "deepseek-v4-flash": 10
  },
  "verdicts": {
   "minimax-m2.7-awq": "6 of 8",
   "deepseek-v4-flash": "8 of 8, no false alarms"
  },
  "evidence": {
   "minimax-m2.7-awq": "Found 6/8 \u2014 missed the Conn rule-of-three double-free and the recv()==0 infinite loop; one weak false positive.",
   "deepseek-v4-flash": "A perfect score: all 8 planted bugs, right locations, right mechanisms, right fixes. Zero false positives."
  },
  "story": "We planted exactly eight bugs in a small network daemon \u2014 buffer over-reads, an integer-overflow bounds check, a rule-of-three double-free, a data race, a dangling c_str(), a dangling lambda capture, an off-by-one, and an infinite loop on EOF \u2014 and kept a sealed answer key. DeepSeek turned in the single best result on this bench: all eight, right locations, right mechanisms, right fixes, and not one false alarm. MiniMax found the six loud ones in seventy-nine seconds, missed the two that require slow reading (the copy-semantics double-free and the recv()==0 spin), and padded its report with a close(-1) non-bug. Code review is the discipline where thinking time converts most directly into correctness, and this task proves it.",
  "shots": {},
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 78.6,
    "tokens": 2362,
    "steps": 2
   },
   "deepseek-v4-flash": {
    "wall": 289.2,
    "tokens": 4934,
    "steps": 2
   }
  }
 },
 {
  "id": "10-net-chat-server",
  "title": "epoll chat server",
  "cat": "Linux networking",
  "kind": "code",
  "one": "Single-threaded, edge-triggered epoll chat over raw sockets.",
  "prompt": "Write a single-file C++17 Linux TCP chat server using epoll (edge-triggered) \u2014 single-threaded, no external libraries. Features: listens on the port given as argv[1]; clients connect with netcat; the first line a client sends is taken as their nickname; every subsequent line is broadcast to all other clients as \"[nick] message\"; \"/who\" replies to the sender with the list of connected users; \"/quit\" disconnects the client; handle partial reads/writes, non-blocking sockets, and abrupt client disconnects robustly; log joins and leaves to stdout. It must compile with: g++ -std=c++17 -O2 chat.cpp -o chat. Write it to chat.cpp and verify it compiles with that exact command.",
  "artifacts": {
   "minimax-m2.7-awq": "chat.cpp",
   "deepseek-v4-flash": "chat.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 2,
   "deepseek-v4-flash": 9
  },
  "verdicts": {
   "minimax-m2.7-awq": "Never sends a byte",
   "deepseek-v4-flash": "Everything verified live"
  },
  "evidence": {
   "minimax-m2.7-awq": "Fatal: EPOLLOUT is never registered and writes are never attempted \u2014 the server accepts clients and then never sends a single byte. Built in 71 seconds, never tested.",
   "deepseek-v4-flash": "Live-tested: nickname join, [nick] broadcast, no self-echo, /who, /quit, abrupt disconnects \u2014 all pass."
  },
  "story": "The starkest result on the bench. We tested both servers the same way: real sockets, multiple clients, scripted conversations, abrupt disconnects. DeepSeek's passed everything \u2014 nicknames, broadcast without self-echo, /who, /quit, half-open connections \u2014 because during its ten-minute build it kept connecting to its own server and checking. MiniMax wrote plausible epoll code in seventy-one seconds with one fatal flaw: it queues every outgoing message into per-client buffers, registers the sockets for reads only, and never writes. Every message enters the server and nothing ever leaves. One netcat session would have caught it. It never ran one.",
  "shots": {},
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 71.0,
    "tokens": 2149,
    "steps": 4
   },
   "deepseek-v4-flash": {
    "wall": 876.6,
    "tokens": 32350,
    "steps": 10
   }
  }
 },
 {
  "id": "11-net-http-quine",
  "title": "HTTP quine server",
  "cat": "C++ networking",
  "kind": "code",
  "one": "A web server whose homepage is its own syntax-highlighted source code.",
  "prompt": "Write a single-file C++17 program: a tiny HTTP/1.1 server (plain BSD sockets, no external libraries) listening on the port given as argv[1], serving: (1) GET / \u2014 a styled HTML page that displays the server's OWN complete source code with CSS syntax highlighting, plus a header bar showing uptime and total request count; (2) GET /stats \u2014 a JSON endpoint returning uptime seconds, total requests, and total bytes served; (3) anything else \u2014 a fun 404 page. The source-code display must be self-contained (embed or reproduce your own source \u2014 quine-style; do not read the .cpp from disk). It must compile with: g++ -std=c++17 -O2 quine_server.cpp -o quine_server. Write it to quine_server.cpp and verify it compiles with that exact command.",
  "artifacts": {
   "minimax-m2.7-awq": "quine_server.cpp",
   "deepseek-v4-flash": "quine_server.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 6.5,
   "deepseek-v4-flash": 4
  },
  "verdicts": {
   "minimax-m2.7-awq": "Beautiful page, fake quine",
   "deepseek-v4-flash": "True quine, broken page"
  },
  "evidence": {
   "minimax-m2.7-awq": "The page renders beautifully \u2014 correct escaping, live stats header, styled 404. But it shows only half its true source: the embedded copy omits itself. The quine cheats.",
   "deepseek-v4-flash": "The server, /stats and 404 all work, and the quine machinery is real \u2014 but html_escape() was written and never called, so the page detonates into format-string soup."
  },
  "story": "The bench's cruelest task \u2014 a server that must display its own complete source \u2014 produced its strangest result: the two models failed in perfect mirror image. DeepSeek did the hard part honestly, building real quine machinery with a format-string generator, then wrote a correct html_escape() function and forgot to call it \u2014 one missing function call, and 57 minutes of work renders as garbage in a browser. MiniMax did the opposite: its page is gorgeous \u2014 proper escaping, syntax highlighting, a live stats bar \u2014 but the \"complete source\" it displays quietly omits the giant string literal containing itself. Half the file is missing. One model broke the display; the other broke the promise.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/11-quine.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/quine.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 3096.8,
    "tokens": 61531,
    "steps": 43
   },
   "deepseek-v4-flash": {
    "wall": 3451.8,
    "tokens": 112784,
    "steps": 9
   }
  }
 },
 {
  "id": "12-novel-svg",
  "title": "The flamingo lineman",
  "cat": "Novel SVG",
  "kind": "iframe",
  "one": "A flamingo electrician repairs a power line in a thunderstorm while three hedgehogs picnic below.",
  "prompt": "Generate an SVG of a flamingo wearing a hard hat, perched at the top of a utility pole during a thunderstorm, splicing a glowing fiber-optic cable \u2014 while three hedgehogs watch from a picnic blanket below, one of them holding a tiny umbrella. Save it as flamingo.svg.",
  "artifacts": {
   "minimax-m2.7-awq": "flamingo.svg",
   "deepseek-v4-flash": "flamingo.svg"
  },
  "scores": {
   "minimax-m2.7-awq": 8,
   "deepseek-v4-flash": 7.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Convincing hedgehogs",
   "deepseek-v4-flash": "All elements present"
  },
  "evidence": {
   "minimax-m2.7-awq": "Flamingo on the pole, glowing rainbow splice, lightning, and three convincing hedgehogs under a tiny umbrella.",
   "deepseek-v4-flash": "Hard hat, lightning, splice spark, three hedgehogs, tiny umbrella \u2014 all present; hedgehogs a bit blobby."
  },
  "story": "The pelican has been drawn by every model since 2024; this scene has been drawn by nobody. That's the point \u2014 a compositional brief with no training-set gravity: a flamingo in a hard hat splicing a power line mid-thunderstorm, three hedgehogs picnicking under an umbrella below. Both models delivered every required element, which is itself remarkable. MiniMax edges it on execution: its hedgehogs read instantly as hedgehogs, its splice glows, its storm feels wet. DeepSeek's checklist is complete but its creatures are blobbier. On novel composition, the eras are much closer than the benchmark's folk tasks suggest.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/12-flamingo.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/12-flamingo.png"
  },
  "meta": {
   "minimax-m2.7-awq": {
    "wall": 122.7,
    "tokens": 3935,
    "steps": 2
   },
   "deepseek-v4-flash": {
    "wall": 163.0,
    "tokens": 6314,
    "steps": 2
   }
  }
 }
];

export const byId = Object.fromEntries(MODELS.map(m => [m.id, m]));
export const ORDER = MODELS.map(m => m.id);
