import StatsSection from "@/components/StatsSection";

export const metadata = {
  title: "Telemetry | Model Showdown by aXite",
  description: "Speed, tokens and wall-clock per task, measured at the API during the runs.",
};

export default function TelemetryPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">Measured at the API / No estimates</p>
          <h1>The telemetry<span>.</span></h1>
          <p className="sub">Timing, tokens, and tool calls captured by a transparent logging proxy during the actual benchmark runs.</p>
        </div>
      </div>
      <StatsSection standalone />
    </main>
  );
}
