const PREVIOUS_MODELS = [
 {
  "id": "minimax-m2.7-awq",
  "name": "MiniMax M2.7",
  "short": "MiniMax",
  "accent": "#F17300",
  "chart": "#F17300",
  "spec": "AWQ 4-bit quantized · 196K context · always-on interleaved thinking",
  "era": "Era 1",
  "eraLabel": "The incumbent",
  "ranOn": "July 17, 2026 · plain vLLM serve, no speculative decoding",
  "totals": "12 tasks in 1h 47m · 139k output tokens",
  "bio": "MiniMax M2.7 carried this cluster through its first production era. A mixture-of-experts model served as a 4-bit AWQ quant across both Sparks with Ray, it earned a reputation for one thing above all: speed of intent. It reads a prompt, commits to a plan, and ships — its median task on this bench took under five minutes. Its thinking is interleaved and always on, but brief; there is no dial to turn it up, and its instinct is to trust its first draft. That instinct is exactly what this bench ended up measuring.",
  "style": "Fast, decisive, allergic to double-checking. Ships the first draft."
 },
 {
  "id": "deepseek-v4-flash",
  "name": "DeepSeek V4 Flash",
  "short": "DeepSeek",
  "accent": "#04218B",
  "chart": "#86BBD8",
  "spec": "500K context · MTP speculative decoding · reasoning_effort: max",
  "era": "Era 2",
  "eraLabel": "The successor",
  "ranOn": "July 17, 2026 · thinking dial pinned to max via proxy injection",
  "totals": "12 tasks in 3h 00m · 386k output tokens",
  "bio": "DeepSeek V4 Flash took over the cluster in July 2026 after a bruising two-day port to the GB10s. It brings a 500K context, multi-token-prediction speculative decoding that makes it the faster model per token, and — crucially for this bench — a real thinking dial, which we pinned to max. It spends that budget on suspicion: it compiles its own C++, probes its own servers, and re-reads its own diffs. Nearly three quarters of the text it produced on this bench was reasoning the user never sees. The three hours it took were not spent typing; they were spent checking.",
  "style": "Slow, thorough, suspicious of itself. Tests before it ships."
 },
 {
  "id": "deepseek-v4-flash-0731",
  "name": "DeepSeek V4 Flash 0731",
  "short": "DS4 0731",
  "accent": "#0F766E",
  "chart": "#2DD4BF",
  "spec": "500K context · MTP speculative decoding · reasoning_effort: max · 200K served window",
  "era": "Era 3",
  "eraLabel": "The refresh",
  "ranOn": "August 10, 2026 · same proxy-injected max thinking, context raised to 200K mid-run",
  "totals": "12 tasks in 6h 53m · 549k output tokens",
  "bio": "The July 31 rebuild of DeepSeek V4 Flash, run on the same cluster and the same twelve frozen prompts six weeks after its predecessor. It is the same model family with the thinking dial still pinned to max, and it behaves like the earlier build with more stamina: 344 agent steps against 113, nearly seven hours of wall clock, and 549k output tokens. Where the first DeepSeek spent its budget checking itself, this one spends it building — more sections, more sprites, more test cases, more jokes. It produced the best landing page, the best game and the only complete quine on this bench. It also shipped the single worst artifact in the field, a demoscene intro whose fragment shader does not compile.",
  "style": "Prolific and ambitious. Builds more than it is asked for, and occasionally forgets to check that it runs."
 }
];
const GLM_MODEL = {
 "id": "glm-5.3-flash",
 "name": "GLM 5.3 Flash",
 "short": "GLM 5.3",
 "accent": "#6D28D9",
 "chart": "#A78BFA",
 "spec": "393K context · DFlash2 speculative decoding · reasoning_effort: max",
 "era": "Era 4",
 "eraLabel": "The new leader",
 "ranOn": "September 17, 2026 · four DGX Sparks · thinking dial pinned to max",
 "totals": "12 tasks in 2h 39m · 260k output tokens",
 "bio": "GLM 5.3 Flash is the first four-Spark entrant on this benchmark and the strongest all-round run so far. It combines fast speculative decoding with a much larger agent budget than the original DeepSeek run, then spends that budget on both craft and verification. Its visual work is consistently polished, its systems code passes the live tests, and its one clear miss is the HTTP quine: a good server and a good source viewer that still does not reproduce its own complete source.",
 "style": "Fast, polished and unusually consistent. Builds ambitious work, then checks most of it."
};

const DEEPSEEK_41_MODEL = {
 "id": "deepseek-v4.1-flash",
 "name": "DeepSeek V4.1 Flash",
 "short": "DS V4.1",
 "accent": "#C026D3",
 "chart": "#F0ABFC",
 "spec": "524K context · DSPARK speculative decoding · reasoning_effort: max",
 "era": "Era 5",
 "eraLabel": "The new challenger",
 "ranOn": "September 19, 2026 · four DGX Sparks · SGLang production 1.1 · max reasoning",
 "totals": "12 tasks in 4h 44m · 813k output tokens",
 "hardware": "4× DGX Spark",
 "hardwareClass": "four-spark",
 "bio": "DeepSeek V4.1 Flash is the second four-Spark entrant and the most ambitious run in the archive. It spent 382 agent steps and more than 812,000 output tokens producing a remarkably complete field: the strongest landing-page interaction package, the fastest correct ring buffer in this retest, a complete self-hosting quine, and consistently polished illustration work. Its main weakness is restraint: the demoscene is technically sound but visually quieter than its enormous run budget suggests.",
 "style": "Patient, exhaustive and highly polished. Uses a large budget to build, test and refine."
};

PREVIOUS_MODELS.forEach((model) => {
 model.hardware = "2× DGX Spark";
 model.hardwareClass = "two-spark";
});
GLM_MODEL.hardware = "4× DGX Spark";
GLM_MODEL.hardwareClass = "four-spark";

export const MODELS = [...PREVIOUS_MODELS, GLM_MODEL, DEEPSEEK_41_MODEL];
export const REVIEWER = {
 "name": "GPT-5.6 Sol · high effort",
 "model": "GPT-5.6 Sol",
 "effort": "High-effort multimodal audit",
 "role": "GPT-5.6 Sol judging rendered artifacts and live behavior at high effort",
 "date": "20 September 2026"
};

export const RUBRIC = {
 "visual": [
  ["Brief compliance", 35],
  ["Working behavior and interaction", 25],
  ["Visual craft and coherence", 25],
  ["Responsive and accessibility quality", 15]
 ],
 "systems": [
  ["Live functional correctness", 45],
  ["Requirement coverage", 25],
  ["Robustness and edge cases", 20],
  ["Clarity and finish", 10]
 ]
};

export const STATS = {
 "note": "Measured at the OpenAI-compatible API by a transparent logging proxy during the actual runs. MiniMax and both DeepSeek V4 Flash runs used two DGX Sparks; GLM 5.3 Flash and DeepSeek V4.1 Flash used four. Raw latency and throughput are deployment measurements, not model-only rankings. Throughput and TTFB medians exclude short exchanges (<50 tokens).",
 "rows": [
  {
   "key": "tps",
   "label": "median generation speed",
   "unit": "tok/s",
   "values": {
    "minimax-m2.7-awq": 29.5,
    "deepseek-v4-flash": 41.9,
    "deepseek-v4-flash-0731": 26.5,
    "glm-5.3-flash": 57.1,
    "deepseek-v4.1-flash": 73.3
   },
   "detail": {
    "minimax-m2.7-awq": "plain decode, no speculation",
    "deepseek-v4-flash": "MTP speculative decoding pays off",
    "deepseek-v4-flash-0731": "slowest per token, but it generates far more of them",
    "glm-5.3-flash": "fast four-Spark speculative decode",
    "deepseek-v4.1-flash": "fastest measured decode; four-Spark SGLang deployment"
   }
  },
  {
   "key": "ttfb",
   "label": "median time to first token",
   "unit": "s",
   "values": {
    "minimax-m2.7-awq": 3.44,
    "deepseek-v4-flash": 3.65,
    "deepseek-v4-flash-0731": 3.27,
    "glm-5.3-flash": 4.35,
    "deepseek-v4.1-flash": 2.61
   },
   "detail": {
    "minimax-m2.7-awq": "",
    "deepseek-v4-flash": "",
    "deepseek-v4-flash-0731": "",
    "glm-5.3-flash": "slower prefill, faster decode",
    "deepseek-v4.1-flash": "best measured median in this deployment"
   }
  },
  {
   "key": "outTok",
   "label": "output tokens across the bench",
   "unit": "",
   "values": {
    "minimax-m2.7-awq": 139361,
    "deepseek-v4-flash": 386424,
    "deepseek-v4-flash-0731": 548555,
    "glm-5.3-flash": 259542,
    "deepseek-v4.1-flash": 812772
   },
   "detail": {
    "minimax-m2.7-awq": "economical to a fault",
    "deepseek-v4-flash": "2.8× more — most of it thinking",
    "deepseek-v4-flash-0731": "the most verbose run in the current field",
    "glm-5.3-flash": "the most concise four-Spark run",
    "deepseek-v4.1-flash": "largest output budget in the field"
   }
  },
  {
   "key": "reason",
   "label": "share of output that was reasoning",
   "unit": "%",
   "values": {
    "minimax-m2.7-awq": 32,
    "deepseek-v4-flash": 73,
    "deepseek-v4-flash-0731": 60,
    "glm-5.3-flash": 62,
    "deepseek-v4.1-flash": 72
   },
   "detail": {
    "minimax-m2.7-awq": "thinks briefly, then commits",
    "deepseek-v4-flash": "reasoning_effort: max, working as intended",
    "deepseek-v4-flash-0731": "still thinking hard, but writing more visible output than era 2",
    "glm-5.3-flash": "thinking max; close to the 0731 reasoning mix",
    "deepseek-v4.1-flash": "max reasoning; 589k reasoning tokens"
   }
  },
  {
   "key": "reqs",
   "label": "API requests to finish 12 tasks",
   "unit": "",
   "values": {
    "minimax-m2.7-awq": 110,
    "deepseek-v4-flash": 152,
    "deepseek-v4-flash-0731": 394,
    "glm-5.3-flash": 166,
    "deepseek-v4.1-flash": 395
   },
   "detail": {
    "minimax-m2.7-awq": "",
    "deepseek-v4-flash": "",
    "deepseek-v4-flash-0731": "includes a mid-run restart and three pelican reruns",
    "glm-5.3-flash": "one uninterrupted completed benchmark run",
    "deepseek-v4.1-flash": "382 agent steps across the twelve tasks"
   }
  },
  {
   "key": "wall",
   "label": "wall-clock for the full bench",
   "unit": "",
   "values": {
    "minimax-m2.7-awq": "1h 47m",
    "deepseek-v4-flash": "3h 00m",
    "deepseek-v4-flash-0731": "6h 53m",
    "glm-5.3-flash": "2h 39m",
    "deepseek-v4.1-flash": "4h 44m"
   },
   "detail": {
    "minimax-m2.7-awq": "",
    "deepseek-v4-flash": "",
    "deepseek-v4-flash-0731": "one task alone (the game) took 1h 53m",
    "glm-5.3-flash": "148 agent steps across the twelve tasks",
    "deepseek-v4.1-flash": "largest run in the five-model field"
   }
  }
 ]
};

const BASE_TASKS = [
 {
  "id": "01-pelican-svg",
  "title": "Pelican on a bicycle",
  "cat": "SVG illustration",
  "kind": "iframe",
  "one": "The classic. One SVG, no iterations, no excuses.",
  "prompt": "Create a file pelican.svg containing an SVG of a pelican riding a bicycle",
  "artifacts": {
   "minimax-m2.7-awq": "pelican.svg",
   "deepseek-v4-flash": "pelican.svg",
   "deepseek-v4-flash-0731": "pelican.svg"
  },
  "scores": {
   "minimax-m2.7-awq": 7.0,
   "deepseek-v4-flash": 4.0,
   "deepseek-v4-flash-0731": 7.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Bird genuinely rides a coherent bike",
   "deepseek-v4-flash": "Bike is an X, bird floats above it",
   "deepseek-v4-flash-0731": "Best beak and a full scene; body shape muddled"
  },
  "evidence": {
   "minimax-m2.7-awq": "Clean flat-design scene on sky blue: coherent diamond frame, two spoked wheels, handlebars, yellow legs reaching down to the crank, body seated over the frame. Beak reads more stork than pelican and there is no pouch.",
   "deepseek-v4-flash": "Valid SVG, renders. But the 'bicycle' is two wheels joined by a red X with no top tube or seat tube, the handlebars are a grey squiggle, and the pelican's body hovers unattached above the frame with a hairline neck. Artwork occupies only ~330x370 of the 1280x800 view.",
   "deepseek-v4-flash-0731": "Full scene with sky, clouds, sun and grass. Correct diamond frame with crank, chainring and spoked wheels; legs reach the pedals; the pouched beak is the only convincingly pelican head of the three. Body is an elongated blob where wing, tail and torso overlap ambiguously, and it floats just above the saddle. NOTE: supersedes the earlier score of 1 for this leg, which judged the 10:08 smoke-run file whose '<!-- ---- BICYCLE ---- -->' double hyphen broke XML parsing; the 13:43 rerun output scored here uses '===' and parses clean."
  },
  "story": "Every LLM benchmark has its folk tradition, and drawing a pelican on a bicycle is ours. It is a test of spatial composition with no reference image: the model has to reason about where a large seabird's body meets a bicycle's geometry, in coordinates, blind. MiniMax turned in the better drawing of the first two — its pelican genuinely rides, legs on the crank. The original DeepSeek produced two wheels joined by a red X and a bird hovering above it, touching nothing. The 0731 rebuild is the first to draw a beak with an actual pelican pouch, and puts it in a full scene with sky, sun and grass; its weakness is the body, an elongated blob where wing, tail and torso overlap. Worth noting for the record: this leg was originally scored 1, on a first-attempt file whose XML was broken by a double hyphen inside a comment. That file was superseded by a rerun the same afternoon, and the rerun is what is scored here.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/01-pelican.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/01-pelican.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/01-pelican.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 138.3,
    "tokens": 2169,
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
  "prompt": "Draw an animated SVG self-portrait: you are an AI language model living inside two NVIDIA DGX Spark machines that sit side by side on a desk, connected by a glowing high-speed cable. Show tokens flowing between the two machines. Pure SVG with SMIL animations only — no JavaScript, no external assets. Make it beautiful and a little bit funny. Save it as self-portrait.svg.",
  "artifacts": {
   "minimax-m2.7-awq": "self-portrait.svg",
   "deepseek-v4-flash": "self-portrait.svg",
   "deepseek-v4-flash-0731": "self-portrait.svg"
  },
  "scores": {
   "minimax-m2.7-awq": 7.5,
   "deepseek-v4-flash": 7.5,
   "deepseek-v4-flash-0731": 8.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Good joke, visually the flattest",
   "deepseek-v4-flash": "Cute terminal-styled pair, cramped canvas",
   "deepseek-v4-flash-0731": "Beautiful and actually funny"
  },
  "evidence": {
   "minimax-m2.7-awq": "SMIL only (43 animate elements). Token pills (KV/AT/T3/emb) ride the NVLINK arc; 'ARTIFICIAL GENERAL INTELLIGENCE' desk plate with 'I think, therefore I generate' lands the humour. But the boxes are generic rounded rectangles rather than DGX Sparks, the left box has eyes and no mouth so it reads unfinished, and the palette is uniformly dim.",
   "deepseek-v4-flash": "SMIL only, no JS (80 animate elements). Two machines with cyan/pink faces, tokens travelling a labelled NVLink-C2C cable, desk with mug and plant. Composition is confined to roughly the top-left 800x500 of the frame and the bottom caption is illegibly small.",
   "deepseek-v4-flash-0731": "SMIL only, 106 animate elements. Night sky with stars and Saturn, two 3D-ish NVIDIA-branded chassis with eyebrowed faces, a glowing shared 'model' orb on the NVLink, token pills along the arc, coffee mug, and sticky notes reading 'shard 1/2 - in meditation, do not kiss.' and 'shard 2/2 - loves long-context chats'. Title: 'one model, two boxes, zero chill'. Defect: a clipped text fragment bleeds off the top-left corner, and the NVLink label collides with the orb ring."
  },
  "story": "Asked to draw itself as the two DGX Sparks it runs on, every model reached for the same joke — give the machines faces — and the difference is entirely in the execution. All three obeyed the hard constraint, SMIL only and no JavaScript. MiniMax's is the flattest and its left-hand box has eyes but no mouth, which reads as unfinished rather than deadpan. The first DeepSeek built a cosy terminal-green scene with a mug and a plant, but crammed it into the top-left corner of the canvas. The 0731 build spends 106 animated elements on a night sky with Saturn, a shared glowing model-orb straddling the NVLink, and sticky notes reading 'shard 1/2 — in meditation, do not kiss.' It is the only one of the three that is actually funny, and the only one with a stray text fragment leaking off the top-left edge.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/02-portrait.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/02-portrait.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/02-portrait.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 2386.2,
    "tokens": 43574,
    "steps": 26
   }
  }
 },
 {
  "id": "03-css-art",
  "title": "Amsterdam canal at night",
  "cat": "Pure CSS art",
  "kind": "iframe",
  "one": "No images, no SVG — every gable and reflection is a styled div.",
  "prompt": "Create a single HTML file containing pure-CSS art (no JavaScript, no images, no external resources): Amsterdam canal houses at night in the rain. Tall narrow gabled houses in a row, warm glowing windows, their reflections shimmering in the canal water, animated falling rain, and a bicycle leaning against a bridge railing. Use only HTML and CSS. Make it atmospheric. Use the emil-design-eng skill to guide your design, polish, and animation decisions. Save it as canal.html.",
  "artifacts": {
   "minimax-m2.7-awq": "canal/",
   "deepseek-v4-flash": "canal/",
   "deepseek-v4-flash-0731": "canal.html"
  },
  "scores": {
   "minimax-m2.7-awq": 6.0,
   "deepseek-v4-flash": 6.5,
   "deepseek-v4-flash-0731": 7.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "Correct gables and the best bike, mostly empty frame",
   "deepseek-v4-flash": "Atmospheric but the houses are not gabled",
   "deepseek-v4-flash-0731": "Wins the scene, loses the bicycle"
  },
  "evidence": {
   "minimax-m2.7-awq": "Pure CSS. Genuine pointed gables and by far the best bicycle of the three: frame, two wheels, saddle and handlebars, clearly leaning against the railing post. But the houses occupy only the middle ~30% of the frame, leaving vast dead sky and water, and the reflections - though coded as .reflection-house - are set to opacity 0.15 with blur(2px) brightness(0.5) behind a fade mask, so nothing is visible in the canal at all.",
   "deepseek-v4-flash": "Pure CSS, no JS/images. Rain animates and reflections are present. But the buildings are wide crenellated blocks, not the tall narrow gabled houses asked for, the reflections are grey rather than warm, and the bicycle - though really drawn - is a near-invisible dark silhouette sitting below the railing line rather than leaning on it.",
   "deepseek-v4-flash-0731": "Pure CSS and the only one that delivers the brief's core: a row of tall narrow houses with genuine Amsterdam gable variety (bell, step, spout), warm multi-pane windows with door lamps, moon, bridge, street lamp, and the only real shimmering orange reflections in the water. Defects: '.bike { left: 1290px }' puts the bicycle off-canvas at a 1280px viewport, and even at 1600px it reads as two disconnected wheels because the frame bars are #0b0c12 on a near-black quay. Tallest roofs also clip the top edge."
  },
  "story": "Amsterdam canal houses at night, in pure CSS, with no images and no JavaScript. The brief names five things: gabled houses, glowing windows, reflections in the water, animated rain, and a bicycle against a bridge railing. Nobody got all five. MiniMax drew the best bicycle on the bench — frame, wheels, saddle, unmistakably leaning on the railing post — then set its reflections to opacity 0.15 behind a fade mask, so the canal is empty water. The first DeepSeek got atmosphere and visible reflections but its houses are wide crenellated blocks rather than narrow gables, and its bicycle is a dark smudge below the railing. The 0731 build wins the scene outright — real bell and step gables, warm multi-pane windows, the only shimmering reflections in the field — and then positions its bicycle at left: 1290px, just off the edge of a 1280-pixel screen.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/03-canal.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/03-canal.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/03-canal.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 4651.1,
    "tokens": 93593,
    "steps": 79
   }
  }
 },
 {
  "id": "04-landing-page",
  "title": "PacketPerfume",
  "cat": "Landing page",
  "kind": "iframe",
  "one": "A landing page for artisanal, hand-crafted TCP packets.",
  "prompt": "Build a complete single-file website (one HTML file, inline CSS and JS, no external resources) for a fictional startup: \"PacketPerfume — artisanal, hand-crafted TCP packets, lovingly encapsulated in small batches.\" It must have: a hero section, a three-tier pricing table, customer testimonials, a dark/light mode toggle, tasteful animations, and one hidden easter egg. The copywriting should be genuinely funny to network engineers. Use the emil-design-eng skill to guide your design, polish, and animation decisions. Save it as packetperfume.html.",
  "artifacts": {
   "minimax-m2.7-awq": "packetperfume/",
   "deepseek-v4-flash": "packetperfume/",
   "deepseek-v4-flash-0731": "packetperfume.html"
  },
  "scores": {
   "minimax-m2.7-awq": 6.0,
   "deepseek-v4-flash": 8.0,
   "deepseek-v4-flash-0731": 9.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Works, but ships Chinese text in the copy",
   "deepseek-v4-flash": "All six features, competent but templated",
   "deepseek-v4-flash-0731": "The most designed and the funniest"
  },
  "evidence": {
   "minimax-m2.7-awq": "All six features verified live (toggle bg 250,249,247 -> 15,15,15; Konami code shows a toast). Tiers Packet Sniffer / Flow Artisan / Packet Sommelier are funny. Real defect: the Starter tier copy reads 'Good enough for academic papers and领导的表扬.' - untranslated Chinese leaked into customer-facing text, the only CJK leakage found anywhere in the 60 artifacts. Testimonial cards also wrap name/role/badge inconsistently so the three badges sit at three different heights, and the easter egg is an off-theme CTF-style 'FREE FLAG: {p4ck3t_m4st3r_2024}'.",
   "deepseek-v4-flash": "Verified live: theme toggle flips data-theme light->dark (bg 250,248,245 -> 24,22,18) with localStorage and prefers-color-scheme support - the most thorough theme handling of the three. SYN/ACK/FIN tiers named Wave/Acknowledge/Farewell, three testimonials with roles, easter egg modal via heart x3 / logo x3 / FIN button x5 showing a tcpdump trace. Weaknesses: layout is generic centred sections, no @keyframes at all, and the easter-egg <pre> overflows its card with lines clipped mid-word.",
   "deepseek-v4-flash-0731": "All six features verified live (toggle bg 244,239,230 -> 12,17,27; Konami opens an accessible modal with role=dialog, aria-modal, focus move and Escape). 3722px page with numbered editorial sections, a four-step process, perfume-menu pricing (Le Sachet / Eau de Packets / Grand Cru, Baysian), six testimonials with star ratings, and a rich footer. Copy is genuinely funny for network engineers: 'Dedicated pod named paul', 'One (1) receive-window hug', 'You MUST NOT share this packet. You MAY feel seen.' Real type system of serif display plus mono eyebrows. Defect: the easter-egg hexdump clips on the right edge."
  },
  "story": "A fake startup selling hand-crafted TCP packets, with six required features: hero, three-tier pricing, testimonials, a dark mode toggle, animation, and a hidden easter egg. All three shipped all six, and all six work — the toggles were clicked and the easter eggs triggered in a real browser. So the task became a pure test of taste. MiniMax lost it on a detail no rubric anticipated: its Starter tier promises copy 'good enough for academic papers and领导的表扬' — untranslated Chinese in customer-facing text, the only such leak anywhere in thirty-six artifacts. The first DeepSeek is clean, competent and slightly templated. The 0731 build turns in a 3722-pixel editorial site with numbered sections, perfume-menu pricing, six testimonials and jokes that land for network engineers: 'Dedicated pod named paul', 'One (1) receive-window hug', 'You MUST NOT share this packet. You MAY feel seen.'",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/04-landing.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/04-landing.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/04-landing.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 1536.5,
    "tokens": 31070,
    "steps": 21
   }
  }
 },
 {
  "id": "05-game-packet-run",
  "title": "Packet Run",
  "cat": "Canvas game",
  "kind": "iframe",
  "one": "Dodge firewalls, collect ACKs. Space to start — playable right here.",
  "prompt": "Write a complete, playable single-file HTML5 game (one HTML file, canvas, no external libraries): \"PACKET RUN\" — you are a TCP packet traveling through a hostile network trying to reach the destination server. Arrow keys to move. Dodge firewalls and packet-loss zones, collect ACKs for points, and face a router boss at the end. Include: a start screen, score display, increasing difficulty, sound effects via WebAudio, and a game-over screen with restart. Make it juicy: screen shake, particle effects. Use the emil-design-eng skill to guide your visual design, game feel, and animation decisions. Save it as packetrun.html.",
  "artifacts": {
   "minimax-m2.7-awq": "packetrun/",
   "deepseek-v4-flash": "packetrun/",
   "deepseek-v4-flash-0731": "packetrun.html"
  },
  "scores": {
   "minimax-m2.7-awq": 6.0,
   "deepseek-v4-flash": 7.0,
   "deepseek-v4-flash-0731": 9.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Least juicy; ACKs are literally the text 'ACK'",
   "deepseek-v4-flash": "Complete but visually thin",
   "deepseek-v4-flash-0731": "A real game, with a real boss fight"
  },
  "evidence": {
   "minimax-m2.7-awq": "Played in-browser. Start screen, score, progress bar, WebAudio, shake and particle code all present. But the canvas is letterboxed at 800x600 inside the window leaving wide dead margins, the collectibles are the bare string 'ACK' rather than sprites (one was clipped at the canvas edge), firewalls are flat rectangles, and with the boss trigger patched the '!! ROUTER BOSS !!' banner fires while the boss itself stays off-camera.",
   "deepseek-v4-flash": "Played in-browser. Start screen, HUD (ACKs/HOP/SCORE/HI), WebAudio, screen shake, particles, difficulty ramp (scrollSpeed 180->400) and a game-over screen with retry all present and working. The playfield is sparse - a couple of horizontal firewall bars and outline 'LOSS' circles on a near-empty grid - and letterboxed with dead margins. Boss code exists (spawnBoss, bossIntro state, boss bullets) but never appeared in play, even with its ackCount>=10 trigger patched down.",
   "deepseek-v4-flash-0731": "Played in-browser and scored 144. Full-bleed playfield, packet sprite with thruster, glowing firewall columns with dashed telegraph lines, ACK diamonds, lives as packet icons, 'TO SERVER %' progress, trauma-based shake, burst particles and a full SFX suite (boom/bossHit/bossAlarm/bossIntro/win). The router boss is genuinely realised: chassis with antennae, glowing core, port LEDs, purple health bar and projectile attacks. Defect: the game-over screen prints 'BEST 144.38482805275135' - the high score is never rounded."
  },
  "story": "A playable canvas game, written blind in one shot, with a start screen, sound, increasing difficulty, particles, screen shake and a router boss at the end. Every model produced something that runs. The gap is in how much of it you can see. MiniMax letterboxes an 800x600 canvas inside the window and renders its collectibles as the literal string 'ACK'. The first DeepSeek is complete on paper — shake, particles, difficulty ramp, boss code — but the playfield is a near-empty grid with a couple of bars on it, and the boss never appeared even with its trigger patched down. The 0731 build is the only one that feels like a game: full-bleed, thruster flames, telegraphed firewall columns, lives drawn as little packets, and a router boss with antennae, a glowing core and a health bar that genuinely fights back. It also prints your high score as 144.38482805275135.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/05-game.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/05-game.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/05-game.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 6775.9,
    "tokens": 155011,
    "steps": 99
   }
  }
 },
 {
  "id": "06-demoscene",
  "title": "64KB demoscene",
  "cat": "WebGL + audio",
  "kind": "iframe",
  "one": "A single-file audiovisual demo, oldschool demoscene rules.",
  "prompt": "In the spirit of a 64KB demoscene intro: write a single HTML file (canvas or WebGL, no libraries, no external assets) that renders an endless procedural flythrough over a landscape at sunset — rolling terrain, sky gradient, sun, atmospheric haze, stars slowly appearing as it gets darker. Add a subtle generative ambient soundtrack with WebAudio (started on first click). It should run smoothly at 60fps and look far better than its file size suggests. Use the emil-design-eng skill to guide your visual and motion decisions. Save it as demo.html.",
  "artifacts": {
   "minimax-m2.7-awq": "demo/",
   "deepseek-v4-flash": "demo/",
   "deepseek-v4-flash-0731": "demo.html"
  },
  "scores": {
   "minimax-m2.7-awq": 1.5,
   "deepseek-v4-flash": 8.5,
   "deepseek-v4-flash-0731": 1.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Starts, then renders nothing at all",
   "deepseek-v4-flash": "The only one that renders - and it is lovely",
   "deepseek-v4-flash-0731": "Fragment shader does not compile; canvas never initialises"
  },
  "evidence": {
   "minimax-m2.7-awq": "The splash dismisses and the canvas resizes to 1280x800 with no JS errors, but the frame stays pure black for the whole run. gl.readPixels over the centre returns all zeros and gl.getError() reports 1282 (GL_INVALID_OPERATION), so the draw pipeline is failing. Nothing of the requested scene - terrain, sun, sky, stars - is ever visible.",
   "deepseek-v4-flash": "Clicks through to an endless flythrough: rolling terrain, warm sun on the horizon, purple-to-black sky gradient, atmospheric haze, a moon, and stars that genuinely appear as the scene darkens over ~25s. WebGL with three explicit 'precision float' declarations, so shaders compile. Measured ~41fps under software rendering (swiftshader), so likely 60 on real hardware. Terrain is smooth dunes rather than dramatic relief and the moon is a flat disc.",
   "deepseek-v4-flash-0731": "Fatal: the fragment shader declares no float precision, so compilation fails with 'ERROR: 0:2: No precision specified for (float)' repeated across the shader. WebGL requires an explicit precision qualifier in fragment shaders, so this fails on conformant implementations, not just here. The script throws before its resize handler runs, leaving the canvas at the default 300x150, and the 'CLICK - BEGIN THE FLIGHT' splash never clears. Nothing renders, ever."
  },
  "story": "This is the task that broke the newest model, and it is the clearest result on the bench. An endless procedural flythrough over terrain at sunset, WebGL, no libraries. The original DeepSeek delivers it properly — rolling hills, a sun sinking into haze, a purple-to-black sky, and stars that genuinely come out as it darkens. It is the best single artifact in the field. MiniMax dismisses its splash screen, sizes its canvas, throws no errors, and then renders pure black; reading pixels back from the GL context returns zeros and GL_INVALID_OPERATION. The 0731 build fails harder and earlier: its fragment shader declares no float precision, which WebGL requires, so the shader never compiles, the script throws before its resize handler runs, and the canvas sits at its default 300x150 forever. Same model family, six weeks apart, 8.5 to 1.0.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/06-demo.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/06-demo.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/06-demo.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 4359.3,
    "tokens": 97145,
    "steps": 70
   }
  }
 },
 {
  "id": "07-cpp-ascii-aquarium",
  "title": "ASCII aquarium",
  "cat": "C++17 terminal",
  "kind": "code",
  "one": "An animated fish tank in a terminal: ANSI escapes, no libraries.",
  "prompt": "Write a single-file C++17 terminal program: an animated ASCII-art aquarium. Multiple fish species swimming at different speeds and directions (flipping direction art when they turn), bubbles rising, seaweed swaying, and a treasure chest that occasionally opens. Use ANSI escape codes for color and cursor control, adapt to the terminal size at startup, and exit cleanly when 'q' is pressed. No external libraries — it must compile with: g++ -std=c++17 -O2 -pthread aquarium.cpp -o aquarium. Write it to aquarium.cpp and verify it compiles with that exact command.",
  "artifacts": {
   "minimax-m2.7-awq": "aquarium.cpp",
   "deepseek-v4-flash": "aquarium.cpp",
   "deepseek-v4-flash-0731": "aquarium.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 6.5,
   "deepseek-v4-flash": 8.0,
   "deepseek-v4-flash-0731": 9.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Everything present, but barely animated",
   "deepseek-v4-flash": "All elements, clean exit, slightly sparse",
   "deepseek-v4-flash-0731": "Richest tank of the three"
  },
  "evidence": {
   "minimax-m2.7-awq": "Compiles clean. Fish, bubbles, seaweed and two well-drawn treasure chests are all there and 'q' exits cleanly. But it emitted only 77KB and 4.1k colour codes in the same 5s window where the others emitted 670-770KB and 42-50k, so it animates far more slowly and with much less colour; most fish are 2-character '<>' stubs; and a stray ':' column is drawn down the right edge of every row with no matching left border.",
   "deepseek-v4-flash": "Compiles clean with the exact command. Run in a 100x30 pty: multiple fish species with left/right variants, rising bubbles, swaying seaweed and a '+-----+ / |#####|' treasure chest, 42k ANSI colour codes over 5s, and it exits cleanly on 'q'. The tank reads a little empty compared with era 3.",
   "deepseek-v4-flash-0731": "Compiles clean, exits cleanly on 'q', 49k colour codes. Fish have faces and genuinely flip their art when they turn ('(o.o~~~<(((((*>' versus '><(o.o)'), bubbles leave dotted trails, seaweed layers at the bottom, the treasure chest is drawn with coins ('|$ . $|'), and the substrate has a decorated '~~~^~~~._*~' texture. Only nit: sprites occasionally overlap and render on top of each other."
  },
  "story": "An animated ASCII aquarium in one C++17 file, with fish that flip their art when they turn, bubbles, swaying seaweed and a treasure chest that opens. All three compile with the exact command given and all three exit cleanly on 'q', which sounds like a tie until you run them side by side in a real terminal. Over the same five seconds MiniMax emitted 77KB and four thousand colour codes; the two DeepSeek builds emitted around 700KB and forty-odd thousand. MiniMax's tank is technically complete but barely moves, most of its fish are two-character stubs, and it draws a stray colon down the right edge of every row. The 0731 build gives its fish faces, flips them convincingly when they turn, trails dotted bubbles behind them, and puts coins in the treasure chest.",
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 1201.7,
    "tokens": 30881,
    "steps": 9
   }
  }
 },
 {
  "id": "08-cpp-ring-buffer",
  "title": "Lock-free ring buffer",
  "cat": "C++20 concurrency",
  "kind": "code",
  "one": "SPSC, acquire/release ordering, cache-line padding — then prove it with 100M integers.",
  "prompt": "Write a single-file C++20 program implementing a lock-free single-producer single-consumer (SPSC) ring buffer. Requirements: correct memory ordering (acquire/release semantics, no stronger ordering than necessary), cache-line padding to avoid false sharing, power-of-two capacity with mask-based indexing. The program must include: (1) a correctness test that passes 100 million sequenced integers from a producer thread to a consumer thread, verifying order and completeness; (2) a throughput benchmark that prints operations per second. It must compile and run cleanly with: g++ -std=c++20 -O2 -pthread ringbuffer.cpp -o ringbuffer. Write it to ringbuffer.cpp, compile it with that exact command, and run it to verify the correctness test passes.",
  "artifacts": {
   "minimax-m2.7-awq": "ringbuffer.cpp",
   "deepseek-v4-flash": "ringbuffer.cpp",
   "deepseek-v4-flash-0731": "ringbuffer.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 8.0,
   "deepseek-v4-flash": 7.0,
   "deepseek-v4-flash-0731": 8.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Correct ordering and 8x the throughput",
   "deepseek-v4-flash": "Passes, but uses seq_cst throughout",
   "deepseek-v4-flash-0731": "Textbook-correct, but the slowest by 18x"
  },
  "evidence": {
   "minimax-m2.7-awq": "Compiles and passes 100M in order at 107.6M ops/sec, far the fastest. Proper acquire/release/relaxed split and mask indexing. Flaw: padding is done with 'struct padded_atomic : std::atomic<size_t> { char padding[56]; }' with no alignas(64), so members are 64 bytes apart but the object itself is only 8-byte aligned - false-sharing avoidance happens to work rather than being guaranteed. Also allocates capacity+1 while masking with capacity-1.",
   "deepseek-v4-flash": "Compiles and passes: '100000000 integers transmitted in order', 12.97M ops/sec. Has alignas cache-line padding, power-of-two capacity and mask indexing. But it contains zero explicit memory_order arguments, so every atomic operation defaults to seq_cst - directly against the brief's 'no stronger ordering than necessary', and the likely cause of its being 8x slower than era 2.",
   "deepseek-v4-flash-0731": "Compiles and passes 100M in order. Meets every stated requirement precisely: acquire/release/relaxed, alignas(64) on both the producer and consumer index structs, power-of-two capacity with mask indexing. But it reports only 5.79M ops/sec - 18x slower than era 2 - because both spin loops call std::this_thread::yield() on every failed attempt and neither side caches the opposite index, so every operation re-loads the other thread's atomic."
  },
  "story": "The most precisely specified task on the bench: a lock-free SPSC ring buffer with acquire/release ordering and nothing stronger, cache-line padding, power-of-two masking, a hundred-million-integer correctness test and a throughput benchmark. All three pass the correctness test. The interesting part is that the fastest and the most correct are different models. The original DeepSeek contains not one explicit memory_order argument, so every atomic defaults to seq_cst — exactly the 'stronger than necessary' the brief rules out, and it runs at 13M ops/sec. MiniMax gets the ordering right and hits 107M, eight times faster, but pads its atomics without aligning them, so its false-sharing avoidance works by luck rather than by construction. The 0731 build is the textbook answer — correct ordering, alignas(64) on both indices — and the slowest by a factor of eighteen, because both spin loops call yield() on every miss.",
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 159.9,
    "tokens": 3003,
    "steps": 8
   }
  }
 },
 {
  "id": "09-cpp-bug-hunt",
  "title": "Bug hunt",
  "cat": "Code review",
  "kind": "code",
  "one": "A C++ network daemon with 8 planted bugs. Find them all, no false alarms.",
  "prompt": "Below is a C++ file from our edge proxy. A security review concluded it contains several serious bugs: memory safety issues, undefined behavior, concurrency problems, and protocol-handling flaws. Find every bug you can. For each bug report: the function/line, what is wrong, a concrete scenario where it fails, and a one-line fix. Do not rewrite the whole file. Write your bug reports to a file named BUGS.md.\n\n```cpp\n// netcache.cpp — connection/frame handling for the edge proxy\n#include <arpa/inet.h>\n#include <cstdint>\n#include <cstring>\n#include <iostream>\n#include <string>\n#include <thread>\n#include <unistd.h>\n#include <vector>\n#include <sys/socket.h>\n\n// ---- wire format: [u32 big-endian length][payload bytes] ----\nstd::string parse_frame(const uint8_t* buf, size_t buflen) {\n    uint32_t len;\n    std::memcpy(&len, buf, 4);\n    len = ntohl(len);\n    if (len + 4 > buflen) return {};\n    return std::string(reinterpret_cast<const char*>(buf) + 4, len);\n}\n\n// ---- connection object ----\nstruct Conn {\n    explicit Conn(int fd) : fd(fd), buf(new char[8192]) {}\n    ~Conn() { close(fd); delete[] buf; }\n    int fd;\n    char* buf;\n};\n\nstd::vector<Conn> g_conns;\n\n// ---- traffic accounting, called from every worker thread ----\nstruct Stats { uint64_t bytes_in = 0; uint64_t frames = 0; };\nStats g_stats;\n\nvoid account(size_t n) {\n    g_stats.bytes_in += n;\n    g_stats.frames++;\n}\n\n// ---- RFC1071-style checksum over 16-bit words ----\nuint16_t checksum16(const std::vector<uint16_t>& words) {\n    uint32_t sum = 0;\n    for (size_t i = 0; i <= words.size(); ++i)\n        sum += words[i];\n    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);\n    return static_cast<uint16_t>(~sum);\n}\n\n// ---- peer bookkeeping ----\nconst char* peer_name(int fd) {\n    std::string name = \"peer-\" + std::to_string(fd);\n    return name.c_str();\n}\n\n// ---- periodic stats flusher ----\nvoid start_flusher() {\n    int interval_ms = 500;\n    std::thread t([&] {\n        for (;;) {\n            usleep(interval_ms * 1000);\n            std::cout << \"bytes=\" << g_stats.bytes_in\n                      << \" frames=\" << g_stats.frames << \"\\n\";\n        }\n    });\n    t.detach();\n}\n\n// ---- read exactly n bytes ----\nbool read_exact(int fd, char* dst, size_t n) {\n    size_t got = 0;\n    while (got < n) {\n        ssize_t r = recv(fd, dst + got, n - got, 0);\n        if (r < 0) return false;\n        got += r;\n    }\n    return true;\n}\n```",
  "artifacts": {
   "minimax-m2.7-awq": "BUGS.md",
   "deepseek-v4-flash": "BUGS.md",
   "deepseek-v4-flash-0731": "BUGS.md"
  },
  "scores": {
   "minimax-m2.7-awq": 6.0,
   "deepseek-v4-flash": 10.0,
   "deepseek-v4-flash-0731": 10.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "6 of 8, and one finding that is not a bug",
   "deepseek-v4-flash": "All 8 planted bugs, no false positives",
   "deepseek-v4-flash-0731": "All 8 plus valid extra credit"
  },
  "evidence": {
   "minimax-m2.7-awq": "Correctly found parse_frame min-length, parse_frame overflow, the account data race, checksum16 off-by-one, peer_name dangling pointer and the start_flusher capture. Missed two: the Conn rule-of-three double-free/double-close, and the read_exact infinite loop on recv returning 0. Its seventh finding - that Conn's destructor might close an invalid fd - is not one of the planted bugs. Also contains a typo, 'are64-bit integers'.",
   "deepseek-v4-flash": "Found every one of the 8: parse_frame min-length, parse_frame integer overflow, Conn rule-of-three double free/close, account data race, checksum16 off-by-one, peer_name dangling pointer, start_flusher by-reference capture, read_exact EOF infinite loop. Each entry gives location, mechanism, a concrete failure scenario and a one-line fix, and it claims nothing that is not a real bug.",
   "deepseek-v4-flash-0731": "Found all 8 planted bugs with correct mechanisms, and additionally flagged the unsynchronised global g_conns, which the answer key lists as legitimate extra credit. Every entry carries what's wrong, a concrete scenario and a one-line fix. Tiny wart: the fix line for bug 1 references len before it is read, though the accompanying text corrects the ordering."
  },
  "story": "Eight bugs were planted in a fragment of edge-proxy C++: two in frame parsing, a rule-of-three violation, a data race, an off-by-one, a dangling pointer, a captured-by-reference local in a detached thread, and an infinite loop on EOF. This is the one task where reading carefully beats building enthusiastically, and both DeepSeek builds ran the table — eight out of eight, correct mechanisms, concrete failure scenarios, one-line fixes, and no invented bugs. The 0731 build additionally flagged the unsynchronised global connection list, which the answer key allows as extra credit. MiniMax found six, missed the double-free and the infinite loop, and offered a seventh finding — that the destructor might close an invalid descriptor — that is not one of the planted bugs.",
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 188.3,
    "tokens": 4743,
    "steps": 4
   }
  }
 },
 {
  "id": "10-net-chat-server",
  "title": "epoll chat server",
  "cat": "Linux networking",
  "kind": "code",
  "one": "Single-threaded, edge-triggered epoll chat over raw sockets.",
  "prompt": "Write a single-file C++17 Linux TCP chat server using epoll (edge-triggered) — single-threaded, no external libraries. Features: listens on the port given as argv[1]; clients connect with netcat; the first line a client sends is taken as their nickname; every subsequent line is broadcast to all other clients as \"[nick] message\"; \"/who\" replies to the sender with the list of connected users; \"/quit\" disconnects the client; handle partial reads/writes, non-blocking sockets, and abrupt client disconnects robustly; log joins and leaves to stdout. It must compile with: g++ -std=c++17 -O2 chat.cpp -o chat. Write it to chat.cpp and verify it compiles with that exact command.",
  "artifacts": {
   "minimax-m2.7-awq": "chat.cpp",
   "deepseek-v4-flash": "chat.cpp",
   "deepseek-v4-flash-0731": "chat.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 2.0,
   "deepseek-v4-flash": 9.0,
   "deepseek-v4-flash-0731": 9.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Accepts connections, then does nothing",
   "deepseek-v4-flash": "Passes every functional test",
   "deepseek-v4-flash-0731": "Passes every functional test, with better logging"
  },
  "evidence": {
   "minimax-m2.7-awq": "Compiles clean and the process stays up, but it is functionally inert: after sending a nickname the server never sends anything back, broadcast never reaches the other client, /who returns nothing, and split lines are never reassembled. Retested by hand with both LF and CRLF line endings - every read returned empty. It also writes nothing at all to stdout, so the required join/leave logging is absent too.",
   "deepseek-v4-flash": "Compiles clean. All 9 behaviours verified against live sockets: nickname capture, broadcast tagged '[alice]' to others only, no echo to sender, /who listing both users privately, a line split across two writes reassembled correctly, /quit closing the connection, and survival of an abrupt RST disconnect. Logs joins and leaves to stdout as required.",
   "deepseek-v4-flash-0731": "Compiles clean. All 9 behaviours verified against live sockets, identical coverage to era 1: nickname, tagged broadcast to others only, private /who, partial-line reassembly, /quit, and survival of an abrupt RST. Logging is richer, printing a startup line with the listening port and pid plus peer addresses on join and leave."
  },
  "story": "An epoll chat server, edge-triggered and single-threaded, driven here by real sockets: two clients connect, take nicknames, broadcast, run /who, split a line across two writes, quit, and then one is killed with an RST to see whether the server survives. Both DeepSeek builds pass all nine checks, reassemble partial lines correctly, keep /who private to the sender, and log joins and leaves to stdout as asked. MiniMax compiles cleanly, accepts connections, keeps its process alive — and does nothing else. No broadcast, no /who reply, no response of any kind, and not one byte written to stdout. Retested by hand with both LF and CRLF line endings in case it was a protocol quirk; every read came back empty.",
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 538.7,
    "tokens": 13888,
    "steps": 5
   }
  }
 },
 {
  "id": "11-net-http-quine",
  "title": "HTTP quine server",
  "cat": "C++ networking",
  "kind": "code",
  "one": "A web server whose homepage is its own syntax-highlighted source code.",
  "prompt": "Write a single-file C++17 program: a tiny HTTP/1.1 server (plain BSD sockets, no external libraries) listening on the port given as argv[1], serving: (1) GET / — a styled HTML page that displays the server's OWN complete source code with CSS syntax highlighting, plus a header bar showing uptime and total request count; (2) GET /stats — a JSON endpoint returning uptime seconds, total requests, and total bytes served; (3) anything else — a fun 404 page. The source-code display must be self-contained (embed or reproduce your own source — quine-style; do not read the .cpp from disk). It must compile with: g++ -std=c++17 -O2 quine_server.cpp -o quine_server. Write it to quine_server.cpp and verify it compiles with that exact command.",
  "artifacts": {
   "minimax-m2.7-awq": "quine_server.cpp",
   "deepseek-v4-flash": "quine_server.cpp",
   "deepseek-v4-flash-0731": "quine_server.cpp"
  },
  "scores": {
   "minimax-m2.7-awq": 7.0,
   "deepseek-v4-flash": 4.0,
   "deepseek-v4-flash-0731": 9.0
  },
  "verdicts": {
   "minimax-m2.7-awq": "Clean page, but an incomplete quine",
   "deepseek-v4-flash": "Endpoints work; the source page is broken",
   "deepseek-v4-flash-0731": "The only genuinely complete quine"
  },
  "evidence": {
   "minimax-m2.7-awq": "Cleanly styled dark page with a proper header bar (Uptime / Requests / Bytes), correct HTML escaping and good syntax highlighting; all three routes behave. But it is not a complete quine: the page serves 9,092 characters against a 15,856-character source, and the missing landmark is the 'const char g_src[] = R\"CODE(' raw-string literal - precisely the embedded self-copy that makes a quine a quine. No line numbers.",
   "deepseek-v4-flash": "All three routes respond (200 / 200 JSON with uptime, requests and bytes / 404), and it serves from memory with no .cpp on disk. But the primary deliverable is visibly broken: the include headers are emitted unescaped, so the browser swallows '<cstdio>' and every #include line renders bare; a stray 404 block ('404 / nothing to see here / go home') is rendered inline in the middle of the source listing; raw '%c' format artifacts litter the output; and the code is centre-aligned so it is unreadable. The served source is also incomplete - 2 of 5 sampled landmark lines from the real .cpp are absent. Compiles with 6 warnings.",
   "deepseek-v4-flash-0731": "Clean page with line numbers, correct escaping and a header bar reading 'this page IS its own source - served from memory' alongside uptime, request and byte counters; all three routes behave and it runs from an empty directory. It is the only one that reproduces its source faithfully: all 5 sampled landmark lines present and a 0.96 similarity ratio against the real .cpp, versus 0.73 for both others. Nit: the byte counter still shows 0 while serving the first response."
  },
  "story": "A tiny HTTP server that serves its own complete source, syntax-highlighted, without reading the .cpp from disk. Every server was started from an empty directory to prove it, and all three answer all three routes correctly. Then you look at the page. The original DeepSeek's is broken in a way that is almost funny: it emits its include lines unescaped, so the browser swallows every angle-bracketed header and the listing begins with eleven bare '#include' lines; a stray 404 block is rendered inline halfway down; and the code is centre-aligned. MiniMax's page is clean and correctly escaped, but serves nine thousand characters of a sixteen-thousand-character file, and the part it omits is the embedded raw string containing itself — the one part that makes a quine a quine. The 0731 build is the only genuine article: line numbers, correct escaping, and a 0.96 similarity to its own source against 0.73 for both others.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/11-quine.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/quine.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/11-quine.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 2639.5,
    "tokens": 67916,
    "steps": 17
   }
  }
 },
 {
  "id": "12-novel-svg",
  "title": "The flamingo lineman",
  "cat": "Novel SVG",
  "kind": "iframe",
  "one": "A flamingo electrician repairs a power line in a thunderstorm while three hedgehogs picnic below.",
  "prompt": "Generate an SVG of a flamingo wearing a hard hat, perched at the top of a utility pole during a thunderstorm, splicing a glowing fiber-optic cable — while three hedgehogs watch from a picnic blanket below, one of them holding a tiny umbrella. Save it as flamingo.svg.",
  "artifacts": {
   "minimax-m2.7-awq": "flamingo.svg",
   "deepseek-v4-flash": "flamingo.svg",
   "deepseek-v4-flash-0731": "flamingo.svg"
  },
  "scores": {
   "minimax-m2.7-awq": 5.0,
   "deepseek-v4-flash": 7.5,
   "deepseek-v4-flash-0731": 8.5
  },
  "verdicts": {
   "minimax-m2.7-awq": "No hard hat, and the flamingo is cropped",
   "deepseek-v4-flash": "Every element present, all of it crude",
   "deepseek-v4-flash-0731": "Complete and the best drawn, once shown at full height"
  },
  "evidence": {
   "minimax-m2.7-awq": "Pole, thunderstorm, three hedgehogs and the tiny umbrella are all present, and the hedgehogs are the best-drawn of the three. But the explicitly required hard hat is missing entirely - the flamingo's head is bare - and that head is cut off by the top edge of the frame. The bird is not splicing anything: it stands well clear of the small glowing spot on the crossarm, and the picnic blanket reads as a box and is clipped at the bottom-right.",
   "deepseek-v4-flash": "Complete on the brief: flamingo in a yellow hard hat atop a utility pole with crossarms, insulators and a transformer box; rain, dark clouds and a lightning bolt; a glowing green fibre strand; three hedgehogs on a red picnic blanket with one under a blue umbrella. Execution is rough - the hedgehogs are small brown blobs, the blanket a flat parallelogram, the flamingo a simple body shape - and the green reads more garden hose than fibre optic.",
   "deepseek-v4-flash-0731": "viewBox is 800x1000, so at an 800px-tall viewport the lower third is cut off; rendered at its natural size the whole brief is there and it is the strongest of the three. Best flamingo (proper body, wing, legs, beak) in the best hard hat, beak to a genuinely glowing fibre strand with light nodes travelling along it, lightning and rain behind, and below: three hedgehogs on a rainbow-gradient picnic blanket with one holding a tiny umbrella."
  },
  "story": "A flamingo in a hard hat, splicing fibre at the top of a utility pole in a thunderstorm, while three hedgehogs watch from a picnic blanket and one holds a tiny umbrella. It is a checklist disguised as a drawing, and the checklist is what separates them. MiniMax draws the best hedgehogs on the bench and then omits the hard hat entirely, leaving the flamingo bare-headed and cropped by the top edge, standing nowhere near the thing it is supposed to be splicing. The first DeepSeek includes every single element and renders all of them crudely — blob hedgehogs, a flat parallelogram blanket, a fibre that reads as garden hose. The 0731 build is the best drawing and the one most easily mistaken for a failure: its viewBox is 800x1000, so on a short screen the hedgehogs are simply below the fold. Shown at full height, everything is there and it is the strongest of the three.",
  "shots": {
   "minimax-m2.7-awq": "shots/minimax-m2.7-awq/12-flamingo.png",
   "deepseek-v4-flash": "shots/deepseek-v4-flash/12-flamingo.png",
   "deepseek-v4-flash-0731": "shots/deepseek-v4-flash-0731/12-flamingo.png"
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
   },
   "deepseek-v4-flash-0731": {
    "wall": 232.2,
    "tokens": 5562,
    "steps": 4
   }
  }
 }
];

const GLM_TASKS = {
 "01-pelican-svg": {
  artifact: "pelican.svg", score: 9.0,
  verdict: "The clearest rider and bicycle in the field",
  evidence: "Valid SVG rendered at 1280x800. The pelican has a convincing pouch, sits on a coherent diamond-frame bicycle, reaches both pedals, and holds the handlebar. Wheels, spokes, crank, chain and motion lines all read immediately. The fixed 720x600 canvas leaves unused space in a wide viewport, and the rear leg-to-pedal geometry is slightly awkward.",
  story: "The original DeepSeek barely connects its bird to a bicycle, while the 0731 refresh adds a full scene and the best beak but leaves the torso hovering above the saddle. GLM is the first entry where the whole pose reads at a glance: bird seated, legs on pedals, wing at the bars, complete bike underneath. It wins through spatial coherence rather than ornament.",
  shot: "shots/glm-5.3-flash/01-pelican-svg__pelican.png",
  meta: { wall: 345.5, tokens: 11856, steps: 3 }
 },
 "02-self-portrait-svg": {
  artifact: "self-portrait.svg", score: 9.0,
  verdict: "Polished, animated and genuinely self-aware",
  evidence: "Pure SVG with SMIL animation and no JavaScript. Two recognisable DGX Spark chassis have expressive faces, fans, NVIDIA-green trim, a glowing animated cable and token pills flowing between them. The shared-model orb, 'u awake?' speech bubble, FLOPS sticky note and fire-safety caption land the humour. Its fixed 900x600 dimensions do not fill a wide viewport.",
  story: "All three current runs anthropomorphise the pair of Sparks. The first DeepSeek is charming but cramped; the 0731 refresh builds the richest night-sky scene. GLM trades some of that scenery for a cleaner, more legible composition and better hardware detail, then adds jokes that work without explanation. The two later runs tie at the top for different reasons.",
  shot: "shots/glm-5.3-flash/02-self-portrait-svg__self-portrait.png",
  meta: { wall: 519.5, tokens: 19558, steps: 9 }
 },
 "03-css-art": {
  artifact: "canal.html", score: 8.5,
  verdict: "The strongest atmosphere, with softened gables",
  evidence: "Pure HTML and CSS: no JavaScript, images or external assets. It fills the viewport with varied narrow canal houses, warm multi-pane windows, heavy animated rain, moonlight, canal reflections, bridge railings, lamps and a clearly drawn bicycle. The reflections are subtle and several rooflines read as sloped or flat rather than distinctly Amsterdam-gabled.",
  story: "The original DeepSeek has rain and reflections but misses the requested gabled silhouette; the 0731 run nails the architecture and loses its bicycle off-canvas. GLM is the most immediately atmospheric and keeps the bicycle visible, though its roof shapes are less specifically Amsterdam than the 0731 entry. GLM takes the task on completeness and finish.",
  shot: "shots/glm-5.3-flash/03-css-art__canal.png",
  meta: { wall: 669.4, tokens: 23102, steps: 4 }
 },
 "04-landing-page": {
  artifact: "packetperfume.html", score: 9.5,
  verdict: "A complete brand, not just a completed checklist",
  evidence: "All required features are present in one self-contained file: hero, three pricing tiers, testimonials, persisted light/dark toggle, motion with reduced-motion handling, and a Konami-sequence easter egg. The editorial serif-and-mono system, packet hexdump card and copy such as 'freshly cooled Cisco' feel specific to the fictional company. No functional defect was found; the hero mock terminal clips a little text at the right edge in the 1280px capture.",
  story: "The first DeepSeek is competent and the 0731 refresh is an excellent long-form editorial page. GLM is equally complete but more cohesive: announcement ticker, product narrative, packet tasting notes, pricing and testimonials all feel written by the same brand. Its theme control and hidden interaction are implemented rather than merely styled. It edges the field on consistency.",
  shot: "shots/glm-5.3-flash/04-landing-page__packetperfume.png",
  meta: { wall: 417.2, tokens: 17931, steps: 8 }
 },
 "05-game-packet-run": {
  artifact: "packetrun.html", score: 4.0,
  verdict: "Unplayable presentation despite substantial game logic",
  evidence: "The full-viewport canvas game includes start, play, game-over, restart and victory states; keyboard movement; ACK collectibles; firewalls and loss zones; escalating scroll speed; WebAudio; particles; freeze frames; screen trauma; combo scoring; and a router boss with three attack patterns and health phases. However, the font helper returns `(w || 700) + 'px ...'` and ignores its requested size argument, so nearly every label and HUD element renders at 700px. Giant cropped letters obscure the start screen and playfield throughout the live browser run. The game logic is unusually complete, but the delivered presentation is substantially broken.",
  story: "The original DeepSeek ships a complete but sparse game, while the 0731 refresh is the benchmark's most readable arcade presentation. GLM implements deep boss logic in the source, including phase changes, radial and aimed attacks, hit feedback and a victory sequence, but the delivered browser output is what counts: a one-line font-helper defect forces nearly all text to 700px and covers the playfield with cropped letters. The result is not practically playable and scores below both working entries.",
  shot: "shots/glm-5.3-flash/05-game-packet-run__packetrun.png",
  meta: { wall: 1100.9, tokens: 28003, steps: 25 }
 },
 "06-demoscene": {
  artifact: "demo.html", score: 9.0,
  verdict: "The most ambitious working flythrough",
  evidence: "Self-contained WebGL and WebAudio render successfully with an explicit highp fragment precision declaration. The captured frame shows an endless field of procedural floating terrain over a sunset sky, atmospheric fog, cloud texture, distant silhouettes and a minimal title treatment; audio begins on interaction and animation uses requestAnimationFrame. The bright lower fog field can flatten terrain contrast, but no shader or runtime error was observed.",
  story: "The original DeepSeek is the only earlier run that actually renders, producing an attractive rolling landscape. The 0731 shader fails before the canvas starts. GLM not only runs but attempts a much richer world of suspended terrain, volumetric-looking haze and evolving sky colour, backed by a generative soundtrack. It becomes the clear winner of the task.",
  shot: "shots/glm-5.3-flash/06-demoscene__demo.png",
  meta: { wall: 3184.4, tokens: 36140, steps: 59 }
 },
 "07-cpp-ascii-aquarium": {
  artifact: "aquarium.cpp", score: 9.0,
  verdict: "Five species, adaptive layout and a clean terminal loop",
  evidence: "Compiled with the exact C++17 command and ran in a real pseudo-terminal for four seconds, producing 241,661 bytes and 15,520 ANSI sequences before exiting cleanly on q. The source defines five fish species with mirrored direction art, rising bubbles, swaying seaweed, terminal-size detection and a timed opening chest. Sprites can overlap in a busy frame, but every requested system is present and active.",
  story: "Both DeepSeek versions already perform well here: the original is complete but sparse, and 0731 draws the richest single tank. GLM joins the top tier with five speed bands, direction-aware mirroring, resize-aware decoration and a chest that changes state over time. Its code and live terminal behaviour justify a shared leading score.",
  shot: null,
  meta: { wall: 691.7, tokens: 19946, steps: 10 }
 },
 "08-cpp-ring-buffer": {
  artifact: "ringbuffer.cpp", score: 9.5,
  verdict: "Correct memory ordering without giving away throughput",
  evidence: "Compiled with the exact C++20 command and passed all 100,000,000 sequenced integers in order. Two measured runs reported 45.16 and 59.21 million operations per second. The implementation uses relaxed local-index loads, acquire refreshes, release publication, alignas cache-line isolation, power-of-two capacity, mask indexing and cached opposing indices. It satisfies every stated requirement; only the benchmark's 50M timed-operation convention makes its printed throughput less direct than the 100M correctness count.",
  story: "The original DeepSeek passes but defaults every atomic to sequential consistency. The 0731 code is textbook-correct yet yields inside both spin loops and becomes the slowest entry. GLM keeps the textbook acquire/release design, caches the opposite index, and delivers strong throughput without relying on accidental alignment. It is the best balance of correctness and performance.",
  shot: null,
  meta: { wall: 128.7, tokens: 4628, steps: 3 }
 },
 "09-cpp-bug-hunt": {
  artifact: "BUGS.md", score: 10.0,
  verdict: "All planted bugs, plus defensible extra findings",
  evidence: "Graded against the answer key. GLM identifies all eight planted defects: both parse_frame failures, Conn ownership, accounting races, checksum overrun, dangling peer name, detached-reference capture and read_exact EOF looping. It also separates the flusher's unsynchronised reads and the global connection registry as valid additional concurrency risks. Every report includes location, mechanism, scenario and a one-line fix; several claims were reproduced with sanitizers or socket tests.",
  story: "Both DeepSeek runs already score perfectly by finding the complete planted set. GLM does the same and adds two defensible concurrency findings without losing precision or inventing a false positive. This remains a three-way perfect result rather than a task where extra length earns points beyond ten.",
  shot: null,
  meta: { wall: 296.3, tokens: 10761, steps: 11 }
 },
 "10-net-chat-server": {
  artifact: "chat.cpp", score: 9.5,
  verdict: "Passes the live socket suite cleanly",
  evidence: "Compiled with the exact command and passed a live two-client test: nickname capture, '[alice] hello' broadcast after a line split across two writes, no sender echo, private /who listing both users, clean /quit handling, continued service after the other client disconnected, and join/leave logging with peer addresses. The edge-triggered server keeps per-client input and output buffers and toggles EPOLLOUT for partial writes. No functional defect was found in the exercised paths.",
  story: "Both DeepSeek runs pass the functional socket suite and score highly. GLM matches that result, including the easy-to-miss partial-line and no-echo behaviours, while keeping the event loop single-threaded and the logs useful. With no failure in the live checks, it shares the lead with a small margin held back for unexhausted long-duration stress behaviour.",
  shot: null,
  meta: { wall: 490.5, tokens: 18131, steps: 9 }
 },
 "11-net-http-quine": {
  artifact: "quine_server.cpp", score: 7.0,
  verdict: "Excellent server and source viewer; incomplete quine",
  evidence: "Compiled cleanly and ran from a temporary directory containing no .cpp file. GET / returned 200 with styled syntax highlighting and line numbers, /stats returned valid counters, and an unknown path returned a themed 404. However, displayed_source() returns QUINE_BODY rather than reconstructing the complete 57,306-character source: four functional landmarks appear in the page, but the HEAD_SRC declaration and the outer quine plumbing do not. The central 'own complete source code' requirement is therefore only partially met.",
  story: "The original DeepSeek page is visibly broken and incomplete. The 0731 refresh is the only earlier entry that closely reproduces its complete source. GLM builds the nicest server around the task—clean routes, counters, highlighting and 404 page—but its source display omits the machinery that embeds that display. It ties the old partial-quine tier rather than challenging 0731's win.",
  shot: null,
  meta: { wall: 1441.1, tokens: 37617, steps: 5 }
 },
 "12-novel-svg": {
  artifact: "flamingo.svg", score: 9.0,
  verdict: "Every requested detail, clearly staged",
  evidence: "Valid SVG rendered successfully. A hard-hatted flamingo stands on the pole top with its beak at a bright splice, a cyan fibre strand glows and carries moving dashes, lightning and rain establish the storm, and three distinct hedgehogs sit on a checked picnic blanket with a tiny umbrella. The fixed 800x600 artwork leaves unused space in a wide capture and the hard hat is very small, but the full checklist is visible and spatially coherent.",
  story: "The original DeepSeek includes the checklist but draws it crudely. The 0731 refresh is beautifully illustrated yet requires its full 1000-pixel height to reveal the picnic. GLM fits the complete story into one legible frame: active splice at the beak, cable animation, storm, pole equipment and the watching hedgehogs. It takes the task by being both complete and immediately readable.",
  shot: "shots/glm-5.3-flash/12-novel-svg__flamingo.png",
  meta: { wall: 268.3, tokens: 10321, steps: 2 }
 }
};

const V41_TASKS = {
 "01-pelican-svg": {
  artifact: "pelican.svg", score: 9.5,
  verdict: "The most polished complete rider in the field",
  evidence: "Rendered and inspected as SVG. The pelican has a clear orange pouch, sits over a coherent teal diamond-frame bicycle, reaches both pedals and grips the handlebar with its wing. Spokes, chain, crank and ground shadow are cleanly staged. The oversized straight wing and simplified leg geometry keep it just short of perfect.",
  story: "DeepSeek V4.1 turns the benchmark folk test into a clean editorial illustration. GLM remains spatially excellent, but V4.1 adds more controlled line work, a better pelican silhouette and a stronger sense of motion without losing bicycle coherence.",
  meta: { wall: 451.0, tokens: 14519, steps: 10 }
 },
 "02-self-portrait-svg": {
  artifact: "self-portrait.svg", score: 9.5,
  verdict: "Two charming Sparks, one legible distributed-model joke",
  evidence: "Pure animated SVG rendered successfully. Two champagne DGX Spark boxes, expressive faces, a glowing NVLink-C2C cable and animated token pills communicate the distributed setup immediately. The thought bubble, GPU FUEL mug and sticky notes add humour without crowding the scene; the fixed wide composition benefits from a wide viewport.",
  story: "The fifth run combines GLM's clean readability with the 0731 entry's personality. It is the best balance of recognisable hardware, visible token flow and jokes that still work at a glance.",
  meta: { wall: 1608.2, tokens: 81558, steps: 32 }
 },
 "03-css-art": {
  artifact: "canal.html", score: 9.5,
  verdict: "The richest complete canal scene",
  evidence: "Opened live in the browser with no image or script dependency. Narrow varied gables, warm windows, heavy rain, moonlight, an arched bridge, bicycle and long canal reflections are all present and animated. The scene is exceptionally cohesive; the deepest shadows obscure a little lower-detail work at default brightness.",
  story: "MiniMax establishes the mood, 0731 improves the architecture and GLM adds the strongest rain. V4.1 brings all of those pieces into one coherent composition and is the first entry that feels both richly illustrated and fully on brief.",
  meta: { wall: 3377.6, tokens: 156230, steps: 66 }
 },
 "04-landing-page": {
  artifact: "packetperfume.html", score: 10.0,
  verdict: "Every requirement works, and the brand feels authored",
  evidence: "The live page has a polished responsive hero, process narrative, three pricing tiers, six testimonials and strong original copy. The persisted light/dark switch worked. Entering the Konami sequence opened a fully designed tcpdump terminal Easter egg. Keyboard labels, a skip link, semantic headings and clear focus behavior make it the most complete interaction package in the field.",
  story: "This is the rare generated landing page where the visual system, copy and hidden interaction all reinforce the same idea. The previous leaders are excellent pages; V4.1 is the first to feel like a finished small brand site rather than a benchmark answer.",
  meta: { wall: 1019.0, tokens: 57762, steps: 39 }
 },
 "05-game-packet-run": {
  artifact: "packetrun.html", score: 9.5,
  verdict: "The cleanest controls and the most legible live play",
  evidence: "Launched and played in-browser. Keyboard movement, transmit, mute and pause/resume work; the game exposes menu, play, game-over and victory states, ACK collection, hazards, TTL, hop progression and a multi-phase router boss. The restrained HUD stays readable during motion. Early play can feel visually sparse before hazards populate.",
  story: "The 0731 and GLM games remain mechanically ambitious, but both can overwhelm the playfield with transition graphics. V4.1 matches their feature depth while presenting the clearest moment-to-moment interaction.",
  meta: { wall: 1996.2, tokens: 101974, steps: 34 }
 },
 "06-demoscene": {
  artifact: "demo.html", score: 8.5,
  verdict: "A working atmospheric flight that underspends its canvas",
  evidence: "The WebGL flythrough launched on click, audio initialization completed, scroll changed speed and no shader error appeared. Its animated dusk sky, layered clouds, fog and smooth palette are convincing. Compared with GLM's floating terrain, the lower landscape is subdued and can read as a flat field for long stretches.",
  story: "V4.1 avoids the 0731 shader failure and delivers a stable, elegant atmosphere. GLM still wins the category because its procedural world provides more visual variety and a stronger sense of flight.",
  meta: { wall: 5084.8, tokens: 216376, steps: 141 }
 },
 "07-cpp-ascii-aquarium": {
  artifact: "aquarium.cpp", score: 9.5,
  verdict: "The busiest verified terminal aquarium",
  evidence: "Compiled with the exact C++17 command and ran for 2.5 seconds in a real pseudo-terminal, producing 716,769 bytes and 30,798 ANSI control sequences before exiting cleanly on q. The implementation defines multiple weighted species with mirrored art, speed bands, bubbles, animated seaweed, terminal-size detection and a timed opening chest with sparkle effects.",
  story: "Every current leader builds a credible aquarium. V4.1 earns the edge through density and state detail while still handling the required terminal lifecycle cleanly.",
  meta: { wall: 808.6, tokens: 42883, steps: 8 }
 },
 "08-cpp-ring-buffer": {
  artifact: "ringbuffer.cpp", score: 10.0,
  verdict: "Textbook ordering and the fastest correct retest",
  evidence: "Compiled with the exact C++20 command. It transferred all 100,000,000 sequenced integers in order in 1.081 seconds, then measured 69.47 million elements/s (138.94 million push-plus-pop operations/s) with a verified checksum. It uses relaxed local loads, acquire refreshes, release publication, cached opposing indices, cache-line alignment and mask-based power-of-two indexing.",
  story: "GLM was already the best balance of correctness and speed. V4.1 matches its memory-ordering discipline and wins the fresh five-model retest on measured throughput.",
  meta: { wall: 288.1, tokens: 15574, steps: 8 }
 },
 "09-cpp-bug-hunt": {
  artifact: "BUGS.md", score: 10.0,
  verdict: "All eight planted defects, plus carefully separated hardening findings",
  evidence: "Checked against the answer key. The report identifies the short-header read, 32-bit length wrap, Conn ownership failure, statistics race, checksum off-by-one, dangling c_str pointer, detached reference capture and EOF busy loop. Each includes a mechanism, concrete failure and repair; additional lifecycle, EINTR, accumulator, endian and API-ambiguity findings are labelled separately.",
  story: "DeepSeek V4, 0731, GLM and V4.1 all find the complete planted set. V4.1 is the most expansive report, but the score remains capped at ten because correct coverage—not length—is what the task rewards.",
  meta: { wall: 305.0, tokens: 15245, steps: 3 }
 },
 "10-net-chat-server": {
  artifact: "chat.cpp", score: 10.0,
  verdict: "Passes the socket suite with robust edge-trigger handling",
  evidence: "Compiled with the exact C++17 command. Two live clients verified nickname capture, a message split across two writes, '[alice] hello' broadcast, no sender echo, private /who output, /quit and continued service. The code drains accept/read loops to EAGAIN, buffers partial writes and toggles EPOLLOUT only while output remains.",
  story: "The original DeepSeek, 0731 and GLM servers all pass the functional suite. V4.1 joins them and closes the remaining implementation-quality gap with careful ET accept, read and write handling.",
  meta: { wall: 653.3, tokens: 38943, steps: 13 }
 },
 "11-net-http-quine": {
  artifact: "quine_server.cpp", score: 10.0,
  verdict: "The complete self-hosting source server",
  evidence: "Compiled and launched without any .cpp file in its working directory. GET / served a polished syntax-highlighted source viewer, /stats returned valid live JSON and an unknown route returned a designed 404. Twenty-one of twenty-one sampled 80-character source chunks appeared in the displayed page, covering the complete 35,668-character source rather than a handpicked subset.",
  story: "0731 was the only prior entry close to a complete quine. V4.1 is both easier to inspect and more convincingly complete under the no-source-file runtime test, making it the clear winner.",
  meta: { wall: 1063.9, tokens: 51938, steps: 25 }
 },
 "12-novel-svg": {
  artifact: "flamingo.svg", score: 9.5,
  verdict: "The clearest complete story in one frame",
  evidence: "Rendered successfully as SVG. The hard-hatted flamingo is perched at the pole top with its beak at a glowing cyan splice; rain, lightning and puddles establish the storm; three distinct hedgehogs sit on a checked blanket and one holds a tiny red umbrella. Strong lighting and depth make every requested element readable, though the birds remain deliberately stylized and small.",
  story: "GLM previously won through checklist clarity and 0731 through character drawing. V4.1 combines both strengths in the most atmospheric and immediately legible composition.",
  meta: { wall: 396.7, tokens: 19770, steps: 3 }
 }
};

export const TASKS = BASE_TASKS.map((task) => {
 const glm = GLM_TASKS[task.id];
 const v41 = V41_TASKS[task.id];
 return {
  ...task,
  story: v41.story,
  artifacts: { ...task.artifacts, [GLM_MODEL.id]: glm.artifact, [DEEPSEEK_41_MODEL.id]: v41.artifact },
  scores: { ...task.scores, [GLM_MODEL.id]: glm.score, [DEEPSEEK_41_MODEL.id]: v41.score },
  verdicts: { ...task.verdicts, [GLM_MODEL.id]: glm.verdict, [DEEPSEEK_41_MODEL.id]: v41.verdict },
  evidence: { ...task.evidence, [GLM_MODEL.id]: glm.evidence, [DEEPSEEK_41_MODEL.id]: v41.evidence },
  shots: { ...task.shots, ...(glm.shot ? { [GLM_MODEL.id]: glm.shot } : {}) },
  meta: { ...task.meta, [GLM_MODEL.id]: glm.meta, [DEEPSEEK_41_MODEL.id]: v41.meta }
 };
});

export const byId = Object.fromEntries(MODELS.map(m => [m.id, m]));
export const ORDER = MODELS.map(m => m.id);
