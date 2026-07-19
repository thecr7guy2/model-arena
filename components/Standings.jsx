"use client";
import { useEffect, useState } from "react";
import { MODELS, TASKS } from "@/lib/data";
import { loadRatings } from "@/lib/ratings";
import Reveal from "./Reveal";

function Bar({ pct, className }) {
  return (
    <div className={`bar ${className || ""}`}>
      <i style={{ width: `${pct}%` }} />
    </div>
  );
}

export default function Standings() {
  const [ratings, setRatings] = useState({});
  useEffect(() => {
    const sync = () => setRatings(loadRatings());
    sync();
    window.addEventListener("ratings-changed", sync);
    return () => window.removeEventListener("ratings-changed", sync);
  }, []);

  return (
    <section className="standings" id="standings">
      <div className="wrap">
        <div className="stand-grid">
          {MODELS.map((m) => {
            const fable = TASKS.reduce((s, t) => s + t.scores[m.id], 0) / TASKS.length;
            const yours = TASKS.map((t) => ratings[t.id]?.[m.id]).filter((v) => v != null);
            const yourAvg = yours.length ? yours.reduce((a, b) => a + b, 0) / yours.length : null;
            return (
              <Reveal className="stand-card" key={m.id} style={{ "--ac": m.accent }}>
                <h3><span className="dot" />{m.name}</h3>
                <div className="bar-row">
                  <div className="lbl"><span>Fable&apos;s average</span><b>{fable.toFixed(1)}</b></div>
                  <Bar pct={fable * 10} />
                </div>
                <div className="bar-row">
                  <div className="lbl"><span>your average</span><b>{yourAvg != null ? yourAvg.toFixed(1) : "—"}</b></div>
                  <Bar pct={yourAvg != null ? yourAvg * 10 : 0} className="you" />
                </div>
                <p className="stand-note">{yours.length}/12 artifacts rated · {m.totals}</p>
              </Reveal>
            );
          })}
        </div>
      </div>
    </section>
  );
}
