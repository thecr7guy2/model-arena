import Link from "next/link";
import { MODELS, TASKS } from "@/lib/data";
import Versus from "@/components/Versus";
import Reveal from "@/components/Reveal";

function resultFor(task) {
  const best = Math.max(...MODELS.map((model) => task.scores[model.id]));
  const winners = MODELS.filter((model) => task.scores[model.id] === best);
  return winners.length === 1 ? winners[0].id : null;
}

export default function Home() {
  const averages = Object.fromEntries(MODELS.map((model) => [
    model.id,
    TASKS.reduce((sum, task) => sum + task.scores[model.id], 0) / TASKS.length,
  ]));
  const wins = Object.fromEntries(MODELS.map((model) => [model.id, 0]));
  TASKS.forEach((task) => {
    const winner = resultFor(task);
    if (winner) wins[winner] += 1;
  });
  const ranking = [...MODELS].sort((a, b) => averages[b.id] - averages[a.id]);
  const leader = ranking[0];
  const lead = ranking.length > 1 ? averages[ranking[0].id] - averages[ranking[1].id] : 0;
  const [first, latest] = MODELS;
  const featured = [TASKS[2], TASKS[4], TASKS[11]];

  return (
    <main className="home">
      <section className="hero">
        <div className="wrap hero-copy">
          <Reveal>
            <p className="eyebrow">An evolving independent model benchmark / July 2026 edition</p>
            <h1>MODEL<br />SHOWDOWN<span className="period">.</span></h1>
            <div className="hero-bottom">
              <p className="lede">Every model faces the same 12 frozen prompts with no retries or cleanup. Inspect the original work, score it yourself, and see how each new model changes the field.</p>
              <div className="hero-actions">
                <Link className="button button-dark" href="/tasks/">Judge the work <span aria-hidden>→</span></Link>
                <Link className="text-link" href="#result">See the result ↓</Link>
              </div>
            </div>
          </Reveal>
        </div>
        <div className="hero-rule" aria-hidden><span>{TASKS.length} FROZEN TASKS</span><span>{TASKS.length * MODELS.length} LIVE ARTIFACTS</span><span>{MODELS.length} CURRENT MODELS</span></div>
      </section>

      <section className="protocol-section" id="method">
        <div className="wrap">
          <Reveal>
            <div className="section-label"><span>00</span> How the benchmark works <b>A growing field, one fixed test</b></div>
            <div className="protocol-intro"><h2>Same test.<br />Every time.</h2><p>Model Showdown is a living benchmark, not a disposable head-to-head. The prompt suite stays frozen. When a new model arrives on the cluster, it runs the same tasks and joins the existing record.</p></div>
            <div className="protocol-steps">
              <article><b>01</b><h3>We freeze the brief</h3><p>Twelve visual, frontend, and systems tasks stay identical across every run.</p></article>
              <article><b>02</b><h3>Models get one attempt</h3><p>No human cleanup, retries, or selective reruns. The generated artifact is the evidence.</p></article>
              <article><b>03</b><h3>You inspect and score</h3><p>Judge before revealing the benchmark verdict. Your ratings stay in your browser.</p></article>
            </div>
            <div className="roster">
              <div className="roster-label">Current roster <span>{MODELS.length} models tested</span></div>
              {MODELS.map((model, index) => <div className="roster-model" key={model.id} style={{ "--ac": model.accent }}><i /><span>0{index + 1}</span><b>{model.name}</b><small>{model.eraLabel}</small><em>{model.ranOn}</em></div>)}
              <div className="roster-next"><span>Next</span><b>Future models join here</b><small>Same prompts. Same scoring. Comparable history.</small></div>
            </div>
          </Reveal>
        </div>
      </section>

      <section className="result-band" id="result">
        <div className="wrap">
          <Reveal>
            <div className="section-label"><span>01</span> The current result <b>Benchmark review score</b></div>
            <div className="scoreboard">
              {MODELS.map((model, index) => (
                <article className="score-side" key={model.id} style={{ "--ac": model.accent }}>
                  <div className="model-index">MODEL {String(index + 1).padStart(2, "0")}</div>
                  <h2>{model.name}</h2>
                  <div className="giant-score">{averages[model.id].toFixed(1)}<small>/10</small></div>
                  <div className="model-facts"><span>{wins[model.id]} wins</span><span>{model.totals.split(" · ")[0]}</span></div>
                </article>
              ))}
            </div>
            <p className="result-callout"><span>{leader.short} leads the current field</span>{ranking.length > 1 ? ` by ${lead.toFixed(1)} points` : ""}, but the aggregate hides the interesting failures.</p>
            <div className="result-tape">
              {TASKS.map((task, index) => {
                const winner = resultFor(task);
                const model = MODELS.find((item) => item.id === winner);
                return <Link key={task.id} href={`/task/${task.id}/`} title={task.title} style={{ "--win": model?.accent || "#f2c94c" }}><b>{String(index + 1).padStart(2, "0")}</b><i /></Link>;
              })}
            </div>
          </Reveal>
        </div>
      </section>

      <section className="exhibit-section">
        <div className="wrap">
          <Reveal>
            <div className="section-label"><span>02</span> Baseline versus newest <b>Drag to compare</b></div>
            <div className="exhibit-title"><h2>A pelican on a bicycle.</h2><p>The benchmark classic exposes spatial reasoning in a single glance. One bird rides. The other floats.</p></div>
            <Versus
              left={`/artifacts/${first.id}/01-pelican-svg/pelican.svg`}
              right={`/artifacts/${latest.id}/01-pelican-svg/pelican.svg`}
              leftLabel={first.short}
              rightLabel={latest.short}
              leftAccent={first.accent}
              rightAccent={latest.accent}
            />
          </Reveal>
        </div>
      </section>

      <section className="field-notes">
        <div className="wrap">
          <Reveal>
            <div className="section-label"><span>03</span> What the averages miss <b>Three decisive tasks</b></div>
            <div className="notes-grid">
              {featured.map((task, index) => {
                const winner = resultFor(task);
                const model = MODELS.find((item) => item.id === winner) || MODELS[index % 2];
                return (
                  <Link className="note-story" href={`/task/${task.id}/`} key={task.id} style={{ "--ac": model.accent }}>
                    <div className="story-image"><img src={`/${task.shots[model.id]}`} alt={`${task.title} by ${model.name}`} /></div>
                    <div className="story-copy"><span>{task.cat} / {task.id.slice(0, 2)}</span><h3>{task.title}</h3><p>{task.verdicts[model.id]}</p><b>Open case →</b></div>
                  </Link>
                );
              })}
            </div>
            <div className="all-tasks-cta"><p>There are nine more cases in the dossier.</p><Link className="button button-dark" href="/tasks/">View all 12 tasks <span>→</span></Link></div>
          </Reveal>
        </div>
      </section>
    </main>
  );
}
