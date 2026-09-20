import "./globals.css";
import Link from "next/link";
import NavLinks from "@/components/NavLinks";
import { MODELS, TASKS } from "@/lib/data";

export const metadata = {
  title: "Model Showdown by aXite Security Tools",
  description:
    `${MODELS.length} locally served models face the same ${TASKS.length} frozen prompts. Compare ${MODELS.length * TASKS.length} artifacts, live tests, hardware disclosures, and GPT-5.6 Sol's high-effort scores.`,
  icons: {
    icon: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'%3E%3Crect width='32' height='32' rx='8' fill='%23fff'/%3E%3Cpath d='M19 3 8 18h7l-3 11 12-15h-8z' fill='%232563eb'/%3E%3C/svg%3E",
  },
};

export default function RootLayout({ children }) {
  return (
    <html lang="en">
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
