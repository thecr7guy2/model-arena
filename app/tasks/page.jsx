import TaskGrid from "@/components/TaskGrid";
import { TASKS, MODELS } from "@/lib/data";

export const metadata = {
  title: "Tasks — Model Arena",
  description: `${TASKS.length} frozen prompts, one shot per model. Every artifact is live.`,
};

export default function TasksPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">The complete case file / {TASKS.length * MODELS.length} preserved artifacts</p>
          <h1>THE TASKS<span>.</span></h1>
          <p className="sub">Every model got the same brief and one uninterrupted attempt. Filter the dossier, open a case, and score each output before seeing the benchmark verdict.</p>
        </div>
      </div>
      <TaskGrid standalone />
    </main>
  );
}
