import Link from "next/link";
import { MODELS, TASKS, REVIEWER } from "@/lib/data";
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
  const first = MODELS[0];
  const latest = MODELS[MODELS.length - 1];

  return (
    <main className="home home-modern">
      <section className="hero hero-modern">
        <div className="wrap">
          <Reveal>
            <div className="hero-modern-grid">
              <div className="hero-message">
                <p className="eyebrow">Independent model benchmark</p>
                <h1>One test.<br />Every model.</h1>
                <p className="lede">Twelve fixed tasks. One uninterrupted attempt. Every artifact preserved so you can see the result, not just the score.</p>
                <div className="hero-actions">
                  <Link className="button button-dark" href="/standings/">See the results <span aria-hidden>→</span></Link>
                  <Link className="text-link" href="/tasks/">Browse all tasks</Link>
                </div>
              </div>
              <aside className="leader-feature" aria-label="Current benchmark leader">
                <span>Current leader</span>
                <strong>{averages[leader.id].toFixed(1)}</strong>
                <h2>{leader.name}</h2>
                <p>{leader.hardware} · {wins[leader.id]} task wins</p>
              </aside>
            </div>
          </Reveal>
          <Reveal className="home-ranking">
            <div className="home-ranking-head"><span>Current field</span><span>{MODELS.length} models · {MODELS.length * TASKS.length} artifacts</span></div>
            {ranking.slice(0, 5).map((model, index) => (
              <Link className="home-rank-row" href="/standings/" key={model.id}>
                <span>{String(index + 1).padStart(2, "0")}</span>
                <div><b>{model.name}</b><small>{model.hardware}</small></div>
                <strong>{averages[model.id].toFixed(1)}</strong>
                <i aria-hidden>→</i>
              </Link>
            ))}
            {MODELS.length > 5 && <Link className="quiet-link" href="/standings/">View all {MODELS.length} models →</Link>}
          </Reveal>
        </div>
      </section>

      <section className="method-modern" id="method">
        <div className="wrap">
          <Reveal>
            <p className="eyebrow">How it stays fair</p>
            <div className="method-title"><h2>Simple rules.<br />Visible evidence.</h2><p>New models join the same benchmark instead of replacing the old comparison. Scores are useful; the original work is the proof.</p></div>
            <div className="method-lines">
              <article><span>01</span><h3>One frozen brief</h3><p>The same visual, frontend and systems tasks for every model.</p></article>
              <article><span>02</span><h3>No cleanup</h3><p>One attempt per task, with the delivered artifact preserved as-is.</p></article>
              <article><span>03</span><h3>One disclosed judge</h3><p>{REVIEWER.model} · {REVIEWER.effort.toLowerCase()}.</p></article>
            </div>
          </Reveal>
        </div>
      </section>

      <section className="exhibit-section">
        <div className="wrap">
          <Reveal>
            <p className="eyebrow">Look beyond the score</p>
            <div className="exhibit-title"><h2>The same prompt.<br />Two very different answers.</h2><p>Drag across the benchmark&apos;s pelican task to compare the first run with the newest.</p></div>
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

      <section className="home-cta"><div className="wrap"><div><p className="eyebrow">Open evidence</p><h2>See what each model actually made.</h2></div><Link className="button button-dark" href="/tasks/">Explore the benchmark <span>→</span></Link></div></section>
    </main>
  );
}
