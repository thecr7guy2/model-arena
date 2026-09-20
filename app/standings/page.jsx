import Standings from "@/components/Standings";
import { REVIEWER } from "@/lib/data";

export const metadata = {
  title: "Results | Model Showdown by aXite",
  description: "GPT-5.6 Sol high-effort scores across all five models and every benchmark task.",
};

export default function StandingsPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">{REVIEWER.name} scorecard / Benchmark review</p>
          <h1>Benchmark results<span>.</span></h1>
          <p className="sub">See average scores from {REVIEWER.name}, task wins, and every case-level result.</p>
        </div>
      </div>
      <Standings standalone />
    </main>
  );
}
