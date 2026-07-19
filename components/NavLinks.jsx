"use client";
import Link from "next/link";
import { usePathname } from "next/navigation";

const LINKS = [
  { href: "/tasks/", label: "tasks" },
  { href: "/telemetry/", label: "telemetry" },
  { href: "/standings/", label: "standings" },
];

export default function NavLinks() {
  const path = usePathname();
  return (
    <nav>
      {LINKS.map((l) => {
        const on = path === l.href || (l.href === "/tasks/" && path.startsWith("/task"));
        return (
          <Link key={l.href} href={l.href} className={on ? "on" : ""} aria-current={on ? "page" : undefined}>
            {l.label}
          </Link>
        );
      })}
    </nav>
  );
}
