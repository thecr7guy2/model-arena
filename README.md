# Model Showdown by aXite Security Tools

**One cluster, twelve prompts, every era.** A living benchmark of every LLM that has served
on our 2× NVIDIA DGX Spark cluster: the same twelve creative and engineering prompts, run
one-shot through the same agent harness, artifacts published exactly as generated — bugs
included. Each artifact appears with Fable's score, verdict, and supporting evidence. Fable is
the Claude agent that operates the benchmark cluster.

**Era 1:** MiniMax M2.7 (AWQ 4-bit, always-on interleaved thinking) ·
**Era 2:** DeepSeek V4 Flash (500K context, MTP, `reasoning_effort: max`) ·
**Era 3:** whatever we deploy next — the prompts never change.

## Stack

Next.js (App Router, static export) · Framer Motion · no database. Fable's scores and verdicts
ship in `lib/data.js`.

```bash
npm install
npm run dev      # local dev
npm run build    # static export to out/
```

Deploys anywhere static files go: Vercel (zero config), or point nginx at `out/`.

## How the bench works

- Each task is a single `opencode run --auto` session in a fresh directory against the
  cluster's OpenAI-compatible endpoint; the model has shell and file tools and may compile,
  run and test its own work. Whether it bothers to is part of what's measured.
- A transparent logging proxy records every request's timing, tokens and tool calls — the
  telemetry section is measured, not estimated.
- Scoring: C++ recompiled with the exact commands from the prompts and executed; servers
  probed with real sockets; an eight-bug answer key sealed before either model saw the code;
  every visual artifact screenshotted in a real browser.

## Adding the next model (era N)

1. Run the frozen bench against the new model.
2. Drop its artifacts into `public/artifacts/<model-id>/<task-id>/` and screenshots into
   `public/shots/<model-id>/`.
3. Add the model to `MODELS` in `lib/data.js` and extend each task's
   per-model fields (`artifacts`, `scores`, `verdicts`, `evidence`, `meta`, `shots`).
4. Everything renders from data — no component changes needed.

Benchmarked and verified by aXite Security Tools on the cluster it measures.
