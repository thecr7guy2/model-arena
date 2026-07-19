import Link from "next/link";
import { MODELS, TASKS } from "@/lib/data";
import Versus from "@/components/Versus";
import Reveal from "@/components/Reveal";

/* Curated highlight moments — headlines from the bench's own results */
const MOMENTS = [
  {
    taskId: "09-cpp-bug-hunt",
    kicker: "the perfect score",
    headline: "8 planted bugs. 8 found. Zero false alarms.",
    modelIdx: 1,
  },
  {
    taskId: "08-cpp-ring-buffer",
    kicker: "the speed upset",
    headline: "109M ops/s — five times its rival's ring buffer.",
    modelIdx: 0,
  },
  {
    taskId: "10-net-chat-server",
    kicker: "the silent server",
    headline: "A chat server that never sends a single byte.",
    modelIdx: 0,
  },
];

function winnerOf(t) {
  let best = null, tie = false;
  for (const m of MODELS) {
    const s = t.scores[m.id];
    if (best === null || s > t.scores[best]) { best = m.id; tie = false; }
    else if (s === t.scores[best]) tie = true;
  }
  return tie ? null : best;
}

export default function Home() {
  const avg = Object.fromEntries(
    MODELS.map((m) => [m.id, TASKS.reduce((s, t) => s + t.scores[m.id], 0) / TASKS.length])
  );
  const wins = Object.fromEntries(MODELS.map((m) => [m.id, 0]));
  const winners = TASKS.map((t) => {
    const w = winnerOf(t);
    if (w) wins[w]++;
    return { t, w };
  });
  const byId = Object.fromEntries(MODELS.map((m) => [m.id, m]));
  const leader = MODELS.reduce((a, b) => (avg[a.id] >= avg[b.id] ? a : b));
  const first = MODELS[0];
  const latest = MODELS[MODELS.length - 1];

  return (
    <main>
      <section className="hero">
        <div className="wrap">
          <Reveal>
            <div className="pill-row" aria-label="Arena rules">
              <span className="pill">12 frozen prompts</span>
              <span className="pill">one shot · no retries</span>
              <span className="pill">artifacts as generated</span>
              <span className="pill">verified live</span>
            </div>
          </Reveal>
          <Reveal delay={0.05}>
            <h1>Same prompts. Every model.<br /><span className="grad">You be the judge.</span></h1>
          </Reveal>

          {/* the scoreboard — what a visitor came to find out */}
          <Reveal delay={0.1}>
            <div className="matchup">
              {MODELS.map((m, i) => (
                <div className={`side ${i > 0 ? "right" : ""}`} key={m.id} style={{ "--ac": m.accent }}>
                  <div className="who">
                    <span className="dot" />
                    <div>
                      <span className="name">{m.name}</span>
                      <span className="era-mini">{m.era} · {m.eraLabel}</span>
                    </div>
                  </div>
                  <div className="score">
                    {avg[m.id].toFixed(1)}
                    <small>/10</small>
                  </div>
                  <span className="winline">{wins[m.id]} task wins</span>
                </div>
              ))}
              <span className="vs" aria-hidden>VS</span>
              <div className="win-strip" aria-label="Task-by-task winners">
                {winners.map(({ t, w }) => (
                  <Link
                    key={t.id}
                    href={`/task/${t.id}/`}
                    className="cell"
                    title={`${t.id.slice(0, 2)} — ${t.title}: ${w ? byId[w].short + " wins" : "tie"}`}
                    style={{
                      background: w
                        ? byId[w].accent
                        : `linear-gradient(135deg, ${MODELS[0].accent} 50%, ${MODELS[MODELS.length - 1].accent} 50%)`,
                    }}
                  />
                ))}
              </div>
              <p className="matchup-note">
                Fable&apos;s scores, task by task — <b style={{ color: leader.accent }}>{leader.short}</b> leads the bench.
                Rate the artifacts to build your own column.
              </p>
            </div>
          </Reveal>

          {/* the moments people talk about */}
          <Reveal delay={0.16}>
            <div className="moments">
              {MOMENTS.map((mo) => {
                const t = TASKS.find((x) => x.id === mo.taskId);
                const m = MODELS[mo.modelIdx];
                return (
                  <Link className="moment" href={`/task/${t.id}/`} key={t.id} style={{ "--ac": m.accent }}>
                    <span className="kicker">{mo.kicker}</span>
                    <p className="line">{mo.headline}</p>
                    <span className="meta">
                      <span className="dot" />{m.short} · {t.title.toLowerCase()} · scored {t.scores[m.id].toFixed(1)}
                    </span>
                  </Link>
                );
              })}
            </div>
          </Reveal>

          {/* the evidence — see one artifact with your own eyes */}
          <Reveal delay={0.2}>
            <div className="proof-head">
              <span className="crumb">exhibit 01 — &ldquo;a pelican riding a bicycle&rdquo;</span>
              <span className="crumb dimmer">drag the seam</span>
            </div>
            <Versus
              left={`/artifacts/${first.id}/01-pelican-svg/pelican.svg`}
              right={`/artifacts/${latest.id}/01-pelican-svg/pelican.svg`}
              leftLabel={first.short.toUpperCase()}
              rightLabel={latest.short.toUpperCase()}
              leftAccent={first.accent}
              rightAccent={latest.accent}
            />
          </Reveal>

          <Reveal delay={0.24}>
            <div className="go-row">
              <Link className="btn primary" href="/tasks/">judge all 12 tasks →</Link>
              <Link className="btn" href="/telemetry/">telemetry</Link>
              <Link className="btn" href="/standings/">standings</Link>
            </div>
          </Reveal>
        </div>
      </section>
    </main>
  );
}
