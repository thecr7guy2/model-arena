import Standings from "@/components/Standings";

export const metadata = {
  title: "Results | Model Showdown by aXite",
  description: "Fable's Claude-based scores across every model and benchmark task.",
};

export default function StandingsPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">Fable scorecard / Claude Opus review</p>
          <h1>Benchmark results<span>.</span></h1>
          <p className="sub">See Fable&apos;s average score for each model, task wins, and every case-level result.</p>
        </div>
      </div>
      <Standings standalone />
    </main>
  );
}
