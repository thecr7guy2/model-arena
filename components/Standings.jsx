"use client";

import Link from "next/link";
import { useMemo, useState } from "react";
import { MODELS, TASKS, REVIEWER, RUBRIC } from "@/lib/data";

export default function Standings() {
  const rows = useMemo(() => {
    const wins = Object.fromEntries(MODELS.map((model) => [model.id, 0]));
    TASKS.forEach((task) => {
      const best = Math.max(...MODELS.map((model) => task.scores[model.id]));
      const winners = MODELS.filter((model) => task.scores[model.id] === best);
      if (winners.length === 1) wins[winners[0].id] += 1;
    });
    return MODELS.map((model) => ({
      model,
      average: TASKS.reduce((sum, task) => sum + task.scores[model.id], 0) / TASKS.length,
      wins: wins[model.id],
    })).sort((a, b) => b.average - a.average);
  }, []);

  const [query, setQuery] = useState("");
  const [selected, setSelected] = useState(() => rows.slice(0, 3).map(({ model }) => model.id));
  const visibleRows = rows.filter(({ model }) => `${model.name} ${model.hardware}`.toLowerCase().includes(query.toLowerCase()));
  const selectedModels = selected.map((id) => MODELS.find((model) => model.id === id)).filter(Boolean);
  const totalScores = TASKS.length * MODELS.length;

  function toggleModel(id) {
    setSelected((current) => current.includes(id)
      ? (current.length === 1 ? current : current.filter((item) => item !== id))
      : (current.length < 3 ? [...current, id] : current));
  }

  return (
    <section className="scorecard-section scorecard-modern">
      <div className="wrap">
        <div className="result-overview">
          <div><span>Leader</span><strong>{rows[0].model.name}</strong><p>{rows[0].average.toFixed(1)} average score</p></div>
          <div><span>Judge</span><strong>{REVIEWER.model}</strong><p>{REVIEWER.effort}</p></div>
          <div><span>Coverage</span><strong>{totalScores}</strong><p>artifacts reviewed</p></div>
        </div>

        <div className="ranking-toolbar">
          <div><h2>Model ranking</h2><p>Select up to three models for the task-by-task view.</p></div>
          <label className="model-search"><span className="sr-only">Search models</span><input value={query} onChange={(event) => setQuery(event.target.value)} placeholder="Search models" /></label>
        </div>

        <div className="ranking-list" role="list">
          {visibleRows.map(({ model, average, wins }) => {
            const isSelected = selected.includes(model.id);
            const selectionFull = selected.length >= 3 && !isSelected;
            const rank = rows.findIndex((row) => row.model.id === model.id) + 1;
            return (
              <article className={`ranking-row ${isSelected ? "selected" : ""}`} key={model.id} role="listitem">
                <span className="rank-number">{String(rank).padStart(2, "0")}</span>
                <div className="rank-model"><h3>{model.name}</h3><p>{model.hardware} · {model.ranOn.split(" · ")[0]}</p></div>
                <div className="rank-wins"><span>Task wins</span><b>{wins}</b></div>
                <strong className="rank-score">{average.toFixed(1)}</strong>
                <button onClick={() => toggleModel(model.id)} disabled={selectionFull} aria-pressed={isSelected}>{isSelected ? "Selected" : selectionFull ? "Limit 3" : "Compare"}</button>
              </article>
            );
          })}
        </div>
        {visibleRows.length === 0 && <p className="ranking-empty">No models match “{query}”.</p>}

        <section className="comparison-section" aria-labelledby="comparison-title">
          <div className="comparison-head">
            <div><span>Focused comparison</span><h2 id="comparison-title">Scores by task</h2></div>
            <div className="selected-models">{selectedModels.map((model) => <button key={model.id} onClick={() => toggleModel(model.id)}>{model.short}<span aria-hidden>×</span></button>)}</div>
          </div>
          <div className="comparison-table" style={{ "--compare-count": selectedModels.length }}>
            <div className="comparison-row comparison-labels"><span>Task</span>{selectedModels.map((model) => <b key={model.id}>{model.short}</b>)}<i /></div>
            {TASKS.map((task) => (
              <Link href={`/task/${task.id}/`} className="comparison-row" key={task.id}>
                <span><small>{task.id.slice(0, 2)}</small>{task.title}</span>
                {selectedModels.map((model) => <b key={model.id}>{task.scores[model.id].toFixed(1)}</b>)}
                <i aria-hidden>→</i>
              </Link>
            ))}
          </div>
        </section>

        <details className="method-details">
          <summary>How scoring works <span>+</span></summary>
          <div><section><h3>Visual tasks</h3><p>{RUBRIC.visual.map(([label, weight]) => `${label} ${weight}%`).join(" · ")}</p></section><section><h3>Systems tasks</h3><p>{RUBRIC.systems.map(([label, weight]) => `${label} ${weight}%`).join(" · ")}</p></section></div>
          <p>Scores measure delivered quality. Hardware-dependent telemetry is disclosed separately and is not folded into the quality average.</p>
        </details>
      </div>
    </section>
  );
}
