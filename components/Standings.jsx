import Link from "next/link";
import { MODELS, TASKS, REVIEWER, RUBRIC } from "@/lib/data";
import Reveal from "./Reveal";

export default function Standings() {
  const wins = Object.fromEntries(MODELS.map((model) => [model.id, 0]));
  TASKS.forEach((task) => {
    const best = Math.max(...MODELS.map((model) => task.scores[model.id]));
    const winners = MODELS.filter((model) => task.scores[model.id] === best);
    if (winners.length === 1) wins[winners[0].id] += 1;
  });

  const rows = MODELS.map((model) => ({
    model,
    average: TASKS.reduce((sum, task) => sum + task.scores[model.id], 0) / TASKS.length,
    wins: wins[model.id],
  }));
  const leader = [...rows].sort((a, b) => b.average - a.average)[0];
  const totalScores = TASKS.length * MODELS.length;

  return (
    <section className="scorecard-section">
      <div className="wrap">
        <Reveal>
          <div className="scorecard-summary">
            <div><span>Reviewer</span><strong>{REVIEWER.model}</strong><p>{REVIEWER.effort} · {REVIEWER.date}</p></div>
            <div><span>Current leader</span><strong>{leader.model.short}</strong><p>{leader.average.toFixed(1)} average score</p></div>
            <div><span>Review coverage</span><strong>{totalScores}<small> / {totalScores}</small></strong><p>rendered or executed artifacts scored</p></div>
          </div>
        </Reveal>

        <Reveal className="rubric-strip">
          <div><span>Visual rubric</span><p>{RUBRIC.visual.map(([label, weight]) => `${label} ${weight}%`).join(" · ")}</p></div>
          <div><span>Systems rubric</span><p>{RUBRIC.systems.map(([label, weight]) => `${label} ${weight}%`).join(" · ")}</p></div>
          <p className="rubric-note">Scores measure delivered quality. Hardware-dependent telemetry is disclosed separately and is not folded into the quality average.</p>
        </Reveal>

        <div className="standing-table">
          <div className="standing-row standing-header"><span>Contender</span><span>{REVIEWER.name} average</span><span>Task wins</span><span>Coverage</span></div>
          {rows.map(({ model, average, wins: taskWins }) => (
            <Reveal className="standing-row" key={model.id} style={{ "--ac": model.accent }}>
              <div className="standing-model"><i /><div><b>{model.name}</b><small>{model.hardware} · {model.style}</small></div></div>
              <strong>{average.toFixed(1)}</strong>
              <strong>{taskWins}</strong>
              <div className="coverage"><span>{TASKS.length} / {TASKS.length}</span><i><b style={{ width: "100%" }} /></i></div>
            </Reveal>
          ))}
        </div>

        <div className="score-matrix-head"><span>Case-by-case score sheet</span><div className="matrix-legend">{MODELS.map((model) => <span key={model.id}><i style={{ background: model.accent }} />{model.short}</span>)}<b>{REVIEWER.name} score</b></div></div>
        <div className="score-matrix">
          {TASKS.map((task) => (
            <Link href={`/task/${task.id}/`} className="matrix-row" key={task.id} style={{ "--model-count": MODELS.length }}>
              <span>{task.id.slice(0, 2)}</span><b>{task.title}</b>
              {MODELS.map((model) => <div key={model.id} style={{ "--ac": model.accent }}><i /><span>{task.scores[model.id].toFixed(1)}</span></div>)}
              <em>→</em>
            </Link>
          ))}
        </div>
      </div>
    </section>
  );
}
