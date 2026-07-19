import TaskDetail from "@/components/TaskDetail";
import { TASKS } from "@/lib/data";

export function generateStaticParams() {
  return TASKS.map((t) => ({ id: t.id }));
}

export async function generateMetadata({ params }) {
  const { id } = await params;
  const t = TASKS.find((x) => x.id === id);
  return t ? { title: `${t.title} — Spark Showdown`, description: t.one } : {};
}

export default async function TaskPage({ params }) {
  const { id } = await params;
  return <TaskDetail taskId={id} />;
}
