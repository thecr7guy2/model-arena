"use client";
import { MODELS, STATS, TASKS, ORDER } from "@/lib/data";
import Reveal from "./Reveal";

function fmtNum(v) {
  if (typeof v === "string") return v;
  return v >= 10000 ? Math.round(v / 1000) + "k" : String(v);
}
function fmtWall(s) {
  return s < 90 ? Math.round(s) + "s" : (s / 60).toFixed(s < 600 ? 1 : 0) + "m";
}

function StatTile({ row, i }) {
  return (
    <Reveal className="stat-tile" delay={Math.min(i * 0.05, 0.2)}>
      <div className="lbl"><span>{String(i + 1).padStart(2, "0")}</span>{row.label}</div>
      <div className="pair">
        {MODELS.map((m) => (
          <div className="half" key={m.id}>
            <div className="num">
              {fmtNum(row.values[m.id])}
              {row.unit && <small>{row.unit}</small>}
            </div>
            <div className="who">
              <i style={{ background: m.chart }} />
              {m.short}
            </div>
          </div>
        ))}
      </div>
      {MODELS.some((model) => row.detail[model.id]) && (
        <p className="note">
          {MODELS.map((model) => row.detail[model.id] ? <span key={model.id}>{model.short}: {row.detail[model.id]}. </span> : null)}
        </p>
      )}
    </Reveal>
  );
}

/** Grouped horizontal bars: one row per task, one thin bar per model.
    Marks use validated chart steps; identity is legend + fixed order, values direct-labeled on hover. */
function GroupedBars({ title, note, valueOf, fmt, max }) {
  return (
    <Reveal className="chart-block">
      <h3>{title}</h3>
      <div className="legend">
        {MODELS.map((m) => (
          <span key={m.id}><i style={{ background: m.chart }} />{m.name}</span>
        ))}
      </div>
      <div className="gchart" role="img" aria-label={title}>
        {TASKS.map((t) => (
          <div className="gc-row" key={t.id}>
            <span className="gc-label">{t.title.toLowerCase()}</span>
            <div className="gc-bars">
              {MODELS.map((m) => {
                const v = valueOf(t, m.id);
                return (
                  <div className="gc-bar" key={m.id} title={`${m.short}: ${fmt(v)}`}>
                    <i style={{ "--bc": m.chart, width: `${(v / max) * 100}%` }} />
                    <span className="gc-val">{fmt(v)}</span>
                  </div>
                );
              })}
            </div>
          </div>
        ))}
      </div>
      {note && <p className="chart-note">{note}</p>}
    </Reveal>
  );
}

export default function StatsSection() {
  const maxWall = Math.max(...TASKS.flatMap((t) => ORDER.map((m) => t.meta[m].wall)));
  const maxTok = Math.max(...TASKS.flatMap((t) => ORDER.map((m) => t.meta[m].tokens)));
  return (
    <section id="telemetry">
      <div className="wrap">
        <div className="telemetry-key" style={{ "--model-count": MODELS.length }}>
          {MODELS.map((model, index) => <div key={model.id} style={{ "--ac": model.accent }}><span>0{index + 1}</span><i /><b>{model.name}</b><small>{model.ranOn}</small></div>)}
        </div>
        <div className="stat-tiles">
          {STATS.rows.map((row, i) => <StatTile key={row.key} row={row} i={i} />)}
        </div>
        <GroupedBars
          title="01 / Wall-clock per task"
          note="The long bars are where a model iterated against its own output."
          valueOf={(t, m) => t.meta[m].wall}
          fmt={fmtWall}
          max={maxWall}
        />
        <GroupedBars
          title="02 / Output tokens per task"
          note="Same prompts, different appetites — reasoning tokens included."
          valueOf={(t, m) => t.meta[m].tokens}
          fmt={(v) => (v >= 1000 ? (v / 1000).toFixed(1) + "k" : v)}
          max={maxTok}
        />
        <p className="chart-note">{STATS.note}</p>
      </div>
    </section>
  );
}
