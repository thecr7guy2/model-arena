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
        <div className="section-head"><h2>Standings</h2></div>
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
        <Reveal className="story">
          <p>
            The pattern behind the numbers: <strong>MiniMax is fast and never looks back.</strong> It finished
            the whole bench in a third of the tokens and a bit over half the time, and its two worst failures —
            a chat server that never sends a byte, a demo behind an eternally black screen — are both
            &ldquo;wrote it, never ran it&rdquo; bugs. One netcat session, one glance at a shader log, and both
            would have been caught.
          </p>
          <p>
            <strong>DeepSeek is slow and suspicious of itself.</strong> It spent its extra ninety minutes
            compiling its own binaries, probing its own servers and re-reading its own diffs — and every task
            on this bench with an objective pass/fail went its way. Its one bad loss, the quine, is the
            exception that defines the rule: the single function it wrote and never called was the single
            thing it couldn&apos;t see without opening a browser.
          </p>
          <p>
            Same prompts, same hardware, same harness. Different temperaments. That&apos;s what this bench
            actually measures — and why every future model that serves on this cluster will face these twelve
            prompts, unchanged.
          </p>
        </Reveal>
      </div>
    </section>
  );
}
