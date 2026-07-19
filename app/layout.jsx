import "./globals.css";
import Link from "next/link";
import NavLinks from "@/components/NavLinks";
import { MODELS, TASKS } from "@/lib/data";
import { Archivo_Black, IBM_Plex_Mono, Inter } from "next/font/google";

const display = Archivo_Black({ subsets: ["latin"], variable: "--font-display", weight: "400" });
const mono = IBM_Plex_Mono({ subsets: ["latin"], variable: "--font-mono", weight: ["400", "500", "600"] });
const body = Inter({ subsets: ["latin"], variable: "--font-body", weight: ["400", "500", "600", "700"] });

export const metadata = {
  title: "Model Showdown by aXite Security Tools",
  description:
    "Every model that serves on this cluster faces the same twelve frozen prompts, one shot each. Compare the original artifacts and Fable's scores.",
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
            <Link className="wordmark" href="/" aria-label="aXite Security Tools Model Showdown home">
              <span className="company-name">aXite Security Tools</span>
              <span className="product-name">Model Showdown</span>
            </Link>
            <NavLinks />
            <Link className="header-cta" href="/standings/">View scorecard <span aria-hidden>↗</span></Link>
          </div>
        </header>
        {children}
        <footer className="site-footer">
          <div className="wrap">
            <span className="footer-brand">aXite Security Tools</span>
            <span>{TASKS.length} frozen prompts. One attempt per model. {TASKS.length * MODELS.length} artifacts preserved in the current field.</span>
          </div>
        </footer>
      </body>
    </html>
  );
}
