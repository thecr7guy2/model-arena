import "./globals.css";
import Link from "next/link";
import { Space_Grotesk, JetBrains_Mono, Inter } from "next/font/google";

const display = Space_Grotesk({ subsets: ["latin"], variable: "--font-display", weight: ["500", "700"] });
const mono = JetBrains_Mono({ subsets: ["latin"], variable: "--font-mono", weight: ["400", "700"] });
const body = Inter({ subsets: ["latin"], variable: "--font-body" });

export const metadata = {
  title: "Spark Showdown — one cluster, twelve prompts, every era",
  description:
    "The same twelve creative and engineering prompts, run one-shot against every LLM that has served on our 2× DGX Spark cluster. Judge the artifacts yourself, then see Fable's verdicts.",
  icons: {
    icon: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'%3E%3Cdefs%3E%3ClinearGradient id='g' x1='0' x2='1'%3E%3Cstop offset='0' stop-color='%23ff7a45'/%3E%3Cstop offset='1' stop-color='%236c8cff'/%3E%3C/linearGradient%3E%3C/defs%3E%3Crect width='32' height='32' rx='7' fill='%230c0f16'/%3E%3Cpath d='M19 3 8 18h7l-3 11 12-15h-8z' fill='url(%23g)'/%3E%3C/svg%3E",
  },
};

export default function RootLayout({ children }) {
  return (
    <html lang="en" className={`${display.variable} ${mono.variable} ${body.variable}`}>
      <body>
        <header className="site-header">
          <div className="wrap header-in">
            <Link className="wordmark" href="/">
              <span className="bolt" />SPARK SHOWDOWN
            </Link>
            <nav>
              <Link href="/#tasks">tasks</Link>
              <Link href="/#telemetry">telemetry</Link>
              <Link href="/#standings">standings</Link>
            </nav>
          </div>
        </header>
        {children}
        <footer className="site-footer">
          <div className="wrap">
            <span>benchmarked, scored &amp; built on the cluster it measures</span>
            <span>prompts frozen · next model: era 3</span>
          </div>
        </footer>
      </body>
    </html>
  );
}
