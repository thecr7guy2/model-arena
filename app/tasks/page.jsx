import TaskGrid from "@/components/TaskGrid";
import { TASKS, MODELS } from "@/lib/data";

export const metadata = {
  title: "Tasks — Model Arena",
  description: "Twelve frozen prompts, one shot each. Every artifact is live.",
};

export default function TasksPage() {
  return (
    <main className="page">
      <div className="wrap">
        <div className="page-head">
          <p className="crumb">{TASKS.length} tasks · {TASKS.length * MODELS.length} artifacts</p>
          <h1>Tasks</h1>
          <p className="sub">One shot per model, artifacts exactly as generated. Open a task to compare and rate.</p>
        </div>
      </div>
      <TaskGrid standalone />
    </main>
  );
}
