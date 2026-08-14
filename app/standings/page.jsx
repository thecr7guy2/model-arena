import Standings from "@/components/Standings";

export const metadata = {
  title: "Results | Model Showdown by aXite",
  description: "Opus's scores across every model and benchmark task.",
};

export default function StandingsPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">Opus scorecard / Benchmark review</p>
          <h1>Benchmark results<span>.</span></h1>
          <p className="sub">See Opus&apos;s average score for each model, task wins, and every case-level result.</p>
        </div>
      </div>
      <Standings standalone />
    </main>
  );
}
