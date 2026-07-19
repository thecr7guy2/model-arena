"use client";
import Link from "next/link";
import { useEffect, useState } from "react";
import { TASKS, MODELS, ORDER } from "@/lib/data";
import { loadRatings } from "@/lib/ratings";
import Reveal from "./Reveal";

export default function TaskGrid() {
  const [ratings, setRatings] = useState({});
  useEffect(() => {
    const sync = () => setRatings(loadRatings());
    sync();
    window.addEventListener("ratings-changed", sync);
    return () => window.removeEventListener("ratings-changed", sync);
  }, []);

  const ratedTotal = TASKS.reduce(
    (n, t) => n + ORDER.filter((m) => ratings[t.id]?.[m] != null).length, 0);

  return (
    <section id="tasks">
      <div className="wrap">
        <div className="section-head">
          <h2>The twelve tasks</h2>
          <span className="count mono">
            {ratedTotal === 0 ? "24 artifacts await your judgment"
              : ratedTotal === 24 ? "all 24 rated — the bench is yours"
              : `${ratedTotal} of 24 artifacts rated`}
          </span>
        </div>
        <p className="section-sub">
          Four visual briefs, four systems-programming briefs, four that live in between. Every artifact below
          is live — the games play, the SVGs animate, the C++ ships with the evidence we gathered running it.
        </p>
        <div className="grid-holder">
          <div className="grid">
            {TASKS.map((t, i) => {
              const rated = ORDER.every((m) => ratings[t.id]?.[m] != null);
              const shotA = t.shots[ORDER[0]], shotB = t.shots[ORDER[1]];
              return (
                <Reveal key={t.id} delay={Math.min((i % 3) * 0.05, 0.15)}>
                  <Link className="card" href={`/task/${t.id}/`}>
                    <div className="thumb">
                      {shotA ? (
                        <>
                          <img src={`/${shotA}`} alt="" loading="lazy" />
                          <img className="b" src={`/${shotB}`} alt="" loading="lazy" />
                        </>
                      ) : (
                        <div className="code-glyph">&gt;_</div>
                      )}
                      <div className="swap-tags">
                        {MODELS.map((m) => <i key={m.id} style={{ background: m.accent }} />)}
                      </div>
                    </div>
                    <div className="card-body">
                      <span className="num">TASK {t.id.slice(0, 2)} / 12</span>
                      <h3>{t.title}</h3>
                      <p className="one">{t.one}</p>
                      <div className="foot">
                        <span className="chip">{t.cat}</span>
                        <span className={`rated-flag ${rated ? "done" : ""}`}>
                          {rated ? "✓ rated" : "unrated"}
                        </span>
                      </div>
                    </div>
                  </Link>
                </Reveal>
              );
            })}
          </div>
        </div>
      </div>
    </section>
  );
}
