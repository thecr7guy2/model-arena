"use client";
import Link from "next/link";
import { useEffect, useState } from "react";
import { AnimatePresence, motion, useReducedMotion } from "framer-motion";
import { TASKS, ORDER, byId } from "@/lib/data";
import { loadRatings, saveRating } from "@/lib/ratings";
import Reveal from "./Reveal";

const EASE = [0.23, 1, 0.32, 1];

function esc(str) {
  return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}
/* single combined pass — replacements can never re-enter injected markup */
function highlight(src, name) {
  const escd = esc(src);
  if (!/\.(cpp|h)$/.test(name)) return escd;
  const re = /(\/\/[^\n]*)|(&quot;(?:[^&\n]|&(?!quot;))*?&quot;)|(^#\w+[^\n]*)|\b(int|void|char|bool|auto|const|constexpr|static|struct|class|return|if|else|while|for|break|continue|new|delete|template|typename|using|namespace|size_t|uint32_t|uint64_t|std)\b/gm;
  return escd.replace(re, (m, c, s, p, k) =>
    c ? `<span class="c">${c}</span>`
    : s ? `<span class="s">${s}</span>`
    : p ? `<span class="p">${p}</span>`
    : `<span class="k">${k}</span>`);
}
function fmtTime(s) {
  return s < 90 ? Math.round(s) + "s" : (s / 60).toFixed(s < 600 ? 1 : 0) + "m";
}
function fmtTokens(n) {
  return n >= 1000 ? (n / 1000).toFixed(1).replace(/\.0$/, "") + "k" : String(n);
}

function SourcePane({ path, hidden }) {
  const [html, setHtml] = useState("loading…");
  useEffect(() => {
    fetch(path)
      .then((r) => (r.ok ? r.text() : Promise.reject(r.status)))
      .then((txt) => setHtml(highlight(txt, path)))
      .catch(() => setHtml("could not load source"));
  }, [path]);
  return (
    <pre
      className={`source ${path.endsWith(".md") ? "wrap" : ""}`}
      hidden={hidden}
      dangerouslySetInnerHTML={{ __html: html }}
    />
  );
}

function Stage({ task, modelId }) {
  const file = task.artifacts[modelId];
  const path = `/artifacts/${modelId}/${task.id}/${file}`;
  const dark = ["06-demoscene", "05-game-packet-run"].includes(task.id);
  const [tab, setTab] = useState(task.kind === "code" && task.shots[modelId] ? "shot" : "src");

  if (task.kind === "iframe") {
    const hint =
      task.id === "05-game-packet-run" ? "click, then Space"
      : task.id === "06-demoscene" ? "click to begin" : null;
    return (
      <div className={`stage ${dark ? "dark" : ""}`}>
        <iframe src={path} loading="lazy" sandbox="allow-scripts allow-same-origin"
          title={`${task.title} — ${byId[modelId].name}`} />
        {hint && <span className="hint">{hint}</span>}
      </div>
    );
  }
  const shot = task.shots[modelId];
  return (
    <>
      <div className="tabs" role="tablist">
        {shot && (
          <button className={`tab ${tab === "shot" ? "on" : ""}`} onClick={() => setTab("shot")}>
            in the browser
          </button>
        )}
        <button className={`tab ${tab === "src" ? "on" : ""}`} onClick={() => setTab("src")}>
          {file}
        </button>
      </div>
      <div className="stage">
        {shot && tab === "shot" && <img className="shot" src={`/${shot}`} alt={`${task.title} rendered`} />}
        <SourcePane path={path} hidden={tab !== "src"} />
      </div>
    </>
  );
}

function Panel({ task, modelId, index }) {
  const m = byId[modelId];
  const meta = task.meta[modelId];
  const [rated, setRated] = useState(null);
  const [value, setValue] = useState(5);
  const reduce = useReducedMotion();
  useEffect(() => {
    const prior = loadRatings()[task.id]?.[modelId];
    if (prior != null) { setRated(prior); setValue(prior); }
  }, [task.id, modelId]);

  const lock = () => { saveRating(task.id, modelId, value); setRated(value); };
  const delta = rated != null ? rated - task.scores[modelId] : 0;

  return (
    <Reveal
      className="panel"
      style={{ "--ac": m.accent }}
      delay={index * 0.07}
    >
      <div className="panel-head">
        <div className="who"><span className="model-code">0{index + 1}</span><span className="name">{m.name}</span></div>
        <span className="era-mini">{m.eraLabel}</span>
      </div>
      <Stage task={task} modelId={modelId} />
      <div className="meta-row">
        <span className="meta-chip">wall <b>{fmtTime(meta.wall)}</b></span>
        <span className="meta-chip">output <b>{fmtTokens(meta.tokens)} tok</b></span>
        <span className="meta-chip">steps <b>{meta.steps}</b></span>
      </div>
      <div className="rate">
        <div className="rate-label"><span>Your score</span><small>0 — 10</small></div>
        <div className="rate-row">
          <input type="range" min="0" max="10" step="0.5" value={value}
            style={{ "--fill": `${value * 10}%` }}
            onChange={(e) => setValue(parseFloat(e.target.value))}
            aria-label={`Your score for ${m.name}`} />
          <span className="val mono">{value.toFixed(1)}</span>
          <button className="button score-button" onClick={lock}>{rated != null ? "Update" : "Reveal verdict"}</button>
        </div>
      </div>
      <AnimatePresence initial={false}>
        {rated != null ? (
          <motion.div
            key="reveal" className="reveal"
            initial={reduce ? false : { height: 0, opacity: 0 }}
            animate={{ height: "auto", opacity: 1 }}
            transition={{ duration: 0.35, ease: EASE }}
          >
            <div className="head">
              <span className="score-num">{task.scores[modelId].toFixed(1)}</span>
              <span className="out-of">/ 10 — benchmark review</span>
              <span className="verdict">{task.verdicts[modelId]}</span>
            </div>
            <p className="evidence">{task.evidence[modelId]}</p>
            <span className="delta">
              {delta === 0
                ? "your score matches the benchmark review"
                : `you scored it ${Math.abs(delta).toFixed(1)} ${delta > 0 ? "higher" : "lower"} than the benchmark review`}
            </span>
          </motion.div>
        ) : (
          <div key="cta" className="reveal-cta">Score the artifact to reveal the benchmark verdict.</div>
        )}
      </AnimatePresence>
    </Reveal>
  );
}

export default function TaskDetail({ taskId }) {
  const t = TASKS.find((x) => x.id === taskId);
  const idx = TASKS.indexOf(t);
  const prev = idx > 0 ? TASKS[idx - 1] : null;
  const next = idx < TASKS.length - 1 ? TASKS[idx + 1] : null;

  return (
    <main className="task-page">
      <div className="wrap">
        <div className="back-row">
          <Link className="back-link" href="/tasks/">← All tasks</Link>
          <div className="task-nav">
            <Link className="nav-square" aria-label="Previous task" aria-disabled={!prev} href={prev ? `/task/${prev.id}/` : "#"}>←</Link>
            <span>{String(idx + 1).padStart(2, "0")} / {TASKS.length}</span>
            <Link className="nav-square" aria-label="Next task" aria-disabled={!next} href={next ? `/task/${next.id}/` : "#"}>→</Link>
          </div>
        </div>
        <div className="detail-head">
          <span className="eyebrow">Case {t.id.slice(0, 2)} / {t.cat}</span>
          <h1>{t.title}</h1>
          <p className="one">{t.one}</p>
        </div>
        <div className="case-context"><span>Field note</span><p>{t.story}</p></div>
        <details className="prompt-box">
          <summary><span>Exact frozen prompt</span><b>Read brief +</b></summary>
          <pre>{t.prompt}</pre>
        </details>
        <div className="case-flow" aria-label="How to judge this task">
          <span><b>01</b> Inspect every original output</span>
          <span><b>02</b> Give each model your score</span>
          <span><b>03</b> Reveal the benchmark review</span>
        </div>
        <div className="panels">
          {ORDER.map((mid, i) => <Panel key={mid} task={t} modelId={mid} index={i} />)}
        </div>
      </div>
    </main>
  );
}
