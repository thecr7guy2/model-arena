"use client";
import { useEffect, useRef } from "react";

/** Scroll-triggered entrance that is safe by construction:
    - server HTML is fully visible (no-JS, crawlers, screenshots all fine)
    - hiding happens only after mount, and only when motion is allowed
    - IntersectionObserver reveals; a timer guarantees reveal regardless */
export default function Reveal({ children, delay = 0, className = "", style, ...rest }) {
  const ref = useRef(null);

  useEffect(() => {
    const el = ref.current;
    if (!el || matchMedia("(prefers-reduced-motion: reduce)").matches) return;
    // only hide if a reveal is plausible soon: element below the fold or IO available
    el.classList.add("rv");
    let done = false;
    let dtm;
    const show = () => {
      if (done) return;
      done = true;
      // stagger via JS timer, not transition-delay — reliable under any clock
      dtm = setTimeout(() => el.classList.add("rv-in"), delay * 1000);
    };
    const io = new IntersectionObserver(
      (entries) => entries.forEach((e) => e.isIntersecting && (show(), io.disconnect())),
      { rootMargin: "-60px" }
    );
    io.observe(el);
    const tm = setTimeout(show, 1100);
    return () => { io.disconnect(); clearTimeout(tm); clearTimeout(dtm); };
  }, [delay]);

  return (
    <div ref={ref} className={className} style={style} {...rest}>
      {children}
    </div>
  );
}
