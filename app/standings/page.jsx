import Standings from "@/components/Standings";

export const metadata = {
  title: "Standings — Model Arena",
  description: "The benchmark review averages against yours, per model.",
};

export default function StandingsPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">Your scorecard / Stored only in this browser</p>
          <h1>YOUR VERDICT<span>.</span></h1>
          <p className="sub">Put your averages beside the benchmark. Scores appear here as you judge each artifact.</p>
        </div>
      </div>
      <Standings standalone />
    </main>
  );
}
