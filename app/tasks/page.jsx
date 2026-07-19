import TaskGrid from "@/components/TaskGrid";
import { TASKS, MODELS } from "@/lib/data";

export const metadata = {
  title: "Tasks | Model Showdown by aXite",
  description: `${TASKS.length} frozen prompts, one shot per model. Every artifact is live.`,
};

export default function TasksPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head dossier-head">
          <p className="eyebrow">The complete case file / {TASKS.length * MODELS.length} preserved artifacts</p>
          <h1>The tasks<span>.</span></h1>
          <p className="sub">Every model received the same brief and one uninterrupted attempt. Open a task, inspect each output, and score it before seeing the benchmark result.</p>
        </div>
      </div>
      <TaskGrid standalone />
    </main>
  );
}
