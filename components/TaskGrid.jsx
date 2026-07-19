"use client";
import Link from "next/link";
import { useEffect, useMemo, useState } from "react";
import { TASKS, MODELS, ORDER, byId } from "@/lib/data";
import { loadRatings } from "@/lib/ratings";
import Reveal from "./Reveal";

const FILTERS = [
  { id: "all", label: "All tasks" },
  { id: "visual", label: "Visual" },
  { id: "code", label: "Code" },
  { id: "unrated", label: "Unrated" },
];

function winnerOf(task) {
  const best = Math.max(...ORDER.map((id) => task.scores[id]));
  const winners = ORDER.filter((id) => task.scores[id] === best);
  return winners.length === 1 ? winners[0] : null;
}

export default function TaskGrid() {
  const [ratings, setRatings] = useState({});
  const [filter, setFilter] = useState("all");
  const [query, setQuery] = useState("");

  useEffect(() => {
    const sync = () => setRatings(loadRatings());
    sync();
    window.addEventListener("ratings-changed", sync);
    return () => window.removeEventListener("ratings-changed", sync);
  }, []);

  const ratedTotal = TASKS.reduce((count, task) => count + ORDER.filter((id) => ratings[task.id]?.[id] != null).length, 0);
  const visible = useMemo(() => TASKS.filter((task) => {
    const matchesQuery = `${task.title} ${task.cat} ${task.one}`.toLowerCase().includes(query.toLowerCase());
    const complete = ORDER.every((id) => ratings[task.id]?.[id] != null);
    const matchesFilter = filter === "all"
      || (filter === "visual" && task.kind === "iframe")
      || (filter === "code" && task.kind === "code")
      || (filter === "unrated" && !complete);
    return matchesQuery && matchesFilter;
  }), [filter, query, ratings]);

  return (
    <section className="task-catalog" id="tasks">
      <div className="wrap">
        <div className="catalog-tools">
          <div className="filter-tabs" role="group" aria-label="Filter tasks">
            {FILTERS.map((item) => <button key={item.id} className={filter === item.id ? "on" : ""} onClick={() => setFilter(item.id)}>{item.label}</button>)}
          </div>
          <label className="task-search"><span>Search</span><input type="search" value={query} onChange={(event) => setQuery(event.target.value)} placeholder="Title or category" /></label>
        </div>
        <div className="catalog-status">
          <span>{visible.length} cases shown</span>
          <span>{ratedTotal} of {TASKS.length * ORDER.length} artifacts scored</span>
          <i><b style={{ width: `${ratedTotal / (TASKS.length * ORDER.length) * 100}%` }} /></i>
        </div>
        <div className="task-grid">
          {visible.map((task, index) => {
            const complete = ORDER.every((id) => ratings[task.id]?.[id] != null);
            const winnerId = winnerOf(task);
            const winner = MODELS.find((model) => model.id === winnerId);
            const visual = task.shots[ORDER[0]];
            return (
              <Reveal key={task.id} delay={Math.min((index % 3) * 0.04, 0.12)}>
                <Link className="task-card" href={`/task/${task.id}/`}>
                  <div className="task-media">
                    {visual ? <img src={`/${task.shots[winnerId || ORDER[0]]}`} alt={`${task.title} benchmark artifact`} loading="lazy" /> : <div className="code-preview"><span>CPP</span><b>&lt;/&gt;</b></div>}
                    <span className="task-number">{task.id.slice(0, 2)}</span>
                    <span className={`rating-state ${complete ? "complete" : ""}`}>{complete ? "Scored" : "Open"}</span>
                  </div>
                  <div className="task-card-copy">
                    <div className="task-type">{task.cat}</div>
                    <h2>{task.title}</h2>
                    <p>{task.one}</p>
                    <div className="card-result">
                      <span>{winner ? <><i style={{ background: winner.accent }} /> {winner.short} leads field</> : "Split decision"}</span>
                      <div className="card-scores">{ORDER.map((id) => <b key={id} title={byId[id].name}><i style={{ background: byId[id].accent }} />{byId[id].short} {task.scores[id].toFixed(1)}</b>)}</div>
                    </div>
                  </div>
                </Link>
              </Reveal>
            );
          })}
        </div>
        {visible.length === 0 && <div className="empty-state"><b>No matching cases.</b><button onClick={() => { setQuery(""); setFilter("all"); }}>Reset filters</button></div>}
      </div>
    </section>
  );
}
