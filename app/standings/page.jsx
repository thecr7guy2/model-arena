import Standings from "@/components/Standings";

export const metadata = {
  title: "Standings — Model Arena",
  description: "Fable's averages against yours, per model.",
};

export default function StandingsPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head">
          <p className="crumb">your ratings live only in this browser</p>
          <h1>Standings</h1>
          <p className="sub">Fable&apos;s average next to yours. Rate artifacts to fill in your side.</p>
        </div>
      </div>
      <Standings standalone />
    </main>
  );
}
