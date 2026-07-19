import StatsSection from "@/components/StatsSection";

export const metadata = {
  title: "Telemetry — Model Arena",
  description: "Speed, tokens and wall-clock per task, measured at the API during the runs.",
};

export default function TelemetryPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head">
          <p className="crumb">measured at the API, not estimated</p>
          <h1>Telemetry</h1>
          <p className="sub">A logging proxy recorded every request during the runs — timing, tokens, tool calls.</p>
        </div>
      </div>
      <StatsSection standalone />
    </main>
  );
}
