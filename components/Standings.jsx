"use client";
import Link from "next/link";
import { useEffect, useMemo, useState } from "react";
import { MODELS, TASKS } from "@/lib/data";
import { clearRatings, loadRatings } from "@/lib/ratings";
import Reveal from "./Reveal";

export default function Standings() {
  const [ratings, setRatings] = useState({});
  useEffect(() => {
    const sync = () => setRatings(loadRatings());
    sync();
    window.addEventListener("ratings-changed", sync);
    return () => window.removeEventListener("ratings-changed", sync);
  }, []);

  const rows = useMemo(() => MODELS.map((model) => {
    const benchmark = TASKS.reduce((sum, task) => sum + task.scores[model.id], 0) / TASKS.length;
    const scores = TASKS.map((task) => ratings[task.id]?.[model.id]).filter((value) => value != null);
    return { model, benchmark, user: scores.length ? scores.reduce((a, b) => a + b, 0) / scores.length : null, count: scores.length };
  }), [ratings]);
  const total = rows.reduce((sum, row) => sum + row.count, 0);

  const reset = () => {
    if (window.confirm("Clear every score saved in this browser?")) clearRatings();
  };

  return (
    <section className="scorecard-section">
      <div className="wrap">
        <Reveal>
          <div className="scorecard-summary">
            <div><span>Progress</span><strong>{total}<small> / {TASKS.length * MODELS.length}</small></strong><p>artifacts judged</p></div>
            <div><span>Your leader</span><strong>{total ? [...rows].sort((a, b) => (b.user || 0) - (a.user || 0))[0].model.short : "—"}</strong><p>{total ? "based on your ratings" : "waiting for a score"}</p></div>
            <div className="scorecard-action"><Link className="button button-dark" href="/tasks/">Continue judging →</Link>{total > 0 && <button onClick={reset}>Clear my scores</button>}</div>
          </div>
        </Reveal>

        <div className="standing-table">
          <div className="standing-row standing-header"><span>Contender</span><span>Benchmark</span><span>Your score</span><span>Coverage</span></div>
          {rows.map(({ model, benchmark, user, count }) => (
            <Reveal className="standing-row" key={model.id} style={{ "--ac": model.accent }}>
              <div className="standing-model"><i /><div><b>{model.name}</b><small>{model.style}</small></div></div>
              <strong>{benchmark.toFixed(1)}</strong>
              <strong>{user == null ? "—" : user.toFixed(1)}</strong>
              <div className="coverage"><span>{count} / {TASKS.length}</span><i><b style={{ width: `${count / TASKS.length * 100}%` }} /></i></div>
            </Reveal>
          ))}
        </div>

        <div className="score-matrix-head"><span>Case-by-case score sheet</span><div className="matrix-legend">{MODELS.map((model) => <span key={model.id}><i style={{ background: model.accent }} />{model.short}</span>)}<b>Benchmark / You</b></div></div>
        <div className="score-matrix">
          {TASKS.map((task) => (
            <Link href={`/task/${task.id}/`} className="matrix-row" key={task.id} style={{ "--model-count": MODELS.length }}>
              <span>{task.id.slice(0, 2)}</span><b>{task.title}</b>
              {MODELS.map((model) => <div key={model.id} style={{ "--ac": model.accent }}><i /><span>{task.scores[model.id].toFixed(1)}</span><span>{ratings[task.id]?.[model.id]?.toFixed(1) || "—"}</span></div>)}
              <em>→</em>
            </Link>
          ))}
        </div>
      </div>
    </section>
  );
}
