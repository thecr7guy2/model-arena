"use client";
import { useEffect, useRef } from "react";
import { useReducedMotion } from "framer-motion";

/** The signature: a draggable clip-path seam between two eras' renders of the same prompt. */
export default function Versus({ left, right, leftLabel, rightLabel, leftAccent, rightAccent }) {
  const ref = useRef(null);
  const reduce = useReducedMotion();

  useEffect(() => {
    const v = ref.current;
    if (!v) return;
    const top = v.querySelector(".pane.top");
    const seam = v.querySelector(".seam");
    const knob = v.querySelector(".knob");
    const set = (p) => {
      const pct = Math.max(2, Math.min(98, p));
      top.style.clipPath = `inset(0 ${100 - pct}% 0 0)`;
      seam.style.left = pct + "%";
      knob.style.left = pct + "%";
    };
    set(50);
    const move = (e) => {
      const r = v.getBoundingClientRect();
      set(((e.clientX - r.left) / r.width) * 100);
    };
    const down = (e) => {
      v.dataset.touched = "1";
      v.setPointerCapture(e.pointerId);
      move(e);
      v.addEventListener("pointermove", move);
    };
    const up = () => v.removeEventListener("pointermove", move);
    v.addEventListener("pointerdown", down);
    v.addEventListener("pointerup", up);
    v.addEventListener("pointercancel", up);

    let raf;
    if (!reduce) {
      let start;
      const sweep = (ts) => {
        if (!start) start = ts;
        const t = (ts - start) / 1400;
        if (t >= 1 || v.dataset.touched) return;
        set(50 + Math.sin(t * Math.PI * 2) * 10);
        raf = requestAnimationFrame(sweep);
      };
      raf = requestAnimationFrame(sweep);
    }
    return () => {
      cancelAnimationFrame(raf);
      v.removeEventListener("pointerdown", down);
      v.removeEventListener("pointerup", up);
      v.removeEventListener("pointercancel", up);
      v.removeEventListener("pointermove", move);
    };
  }, [reduce]);

  return (
    <div className="versus" ref={ref} aria-label="Drag to compare the two models">
      <div className="pane"><img src={right} alt={rightLabel} /></div>
      <div className="pane top"><img src={left} alt={leftLabel} /></div>
      <div className="seam" style={{ left: "50%" }} />
      <div className="knob" style={{ left: "50%" }}>&lt;&gt;</div>
      <span className="tag left" style={leftAccent ? { color: leftAccent } : undefined}>{leftLabel}</span>
      <span className="tag right" style={rightAccent ? { color: rightAccent } : undefined}>{rightLabel}</span>
    </div>
  );
}
