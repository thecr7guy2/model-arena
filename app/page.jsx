import { MODELS } from "@/lib/data";
import Versus from "@/components/Versus";
import Reveal from "@/components/Reveal";
import StatsSection from "@/components/StatsSection";
import TaskGrid from "@/components/TaskGrid";
import Standings from "@/components/Standings";

export default function Home() {
  // the slider compares the first era against the latest one
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
          <Reveal delay={0.06}>
            <h1>
              Same prompts. Every model.<br />
              <span className="grad">You be the judge.</span>
            </h1>
          </Reveal>
          <Reveal delay={0.12}>
            <Versus
              left={`/artifacts/${first.id}/01-pelican-svg/pelican.svg`}
              right={`/artifacts/${latest.id}/01-pelican-svg/pelican.svg`}
              leftLabel={first.short.toUpperCase()}
              rightLabel={latest.short.toUpperCase()}
              leftAccent={first.accent}
              rightAccent={latest.accent}
            />
            <p className="versus-hint">task 01 — &ldquo;a pelican riding a bicycle&rdquo; · same prompt, drag the seam</p>
          </Reveal>
          <Reveal delay={0.18}>
            <div className="model-strip">
              {MODELS.map((m) => (
                <div className="model-row" key={m.id} style={{ "--ac": m.accent }}>
                  <span className="dot" />
                  <span className="name">{m.name}</span>
                  <span className="era-mini">{m.era} · {m.eraLabel}</span>
                  <div className="spec-chips">
                    {m.spec.split("·").map((s) => <span key={s}>{s.trim()}</span>)}
                    <span>{m.totals}</span>
                  </div>
                </div>
              ))}
            </div>
          </Reveal>
        </div>
      </section>

      <TaskGrid />
      <StatsSection />
      <Standings />
    </main>
  );
}
