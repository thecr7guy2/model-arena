import { MODELS, ORDER } from "@/lib/data";
import Versus from "@/components/Versus";
import Reveal from "@/components/Reveal";
import StatsSection from "@/components/StatsSection";
import TaskGrid from "@/components/TaskGrid";
import Standings from "@/components/Standings";

export default function Home() {
  return (
    <main>
      <section className="hero">
        <div className="wrap">
          <Reveal>
            <p className="eyebrow"><b>2× NVIDIA DGX SPARK</b> · 169.254.x.x cluster link · one prompt set, frozen forever</p>
          </Reveal>
          <Reveal delay={0.06}>
            <h1>
              Every model that runs on this cluster faces the same twelve prompts.{" "}
              <span className="m1">MiniMax</span> came first. <span className="m2">DeepSeek</span> took over.
            </h1>
          </Reveal>
          <Reveal delay={0.12}>
            <p className="lede">
              One shot per task, a fresh session every time, artifacts kept exactly as generated —{" "}
              <strong>nothing here was retried, fixed, or cleaned up.</strong> Drag the seam to compare two
              eras of the same machine, then judge the work yourself. Your scores unlock ours.
            </p>
          </Reveal>
          <Reveal delay={0.18}>
            <Versus
              left={`/artifacts/${ORDER[0]}/01-pelican-svg/pelican.svg`}
              right={`/artifacts/${ORDER[1]}/01-pelican-svg/pelican.svg`}
              leftLabel="MINIMAX M2.7"
              rightLabel="DEEPSEEK V4"
            />
            <p className="versus-hint">task 01 — &ldquo;a pelican riding a bicycle&rdquo; · same prompt, two eras · drag the seam</p>
          </Reveal>

          <div className="eras">
            {MODELS.map((m, i) => (
              <Reveal className="era-card" key={m.id} style={{ "--ac": m.accent }} delay={i * 0.07}>
                <span className="era-tag">{m.era} · {m.eraLabel}</span>
                <h3>{m.name}</h3>
                <p className="spec">{m.spec}</p>
                <p className="bio">{m.bio}</p>
                <p className="style-line">&ldquo;{m.style}&rdquo;</p>
                <p className="fine">{m.ranOn}<br />{m.totals}</p>
              </Reveal>
            ))}
          </div>

          <Reveal className="rules">
            <div className="rule"><div className="k">one shot</div><div className="v">Each task is a single <em>opencode run --auto</em> session. No retries, no human edits.</div></div>
            <div className="rule"><div className="k">frozen prompts</div><div className="v">The twelve prompts <em>never change</em> — every future model faces them too.</div></div>
            <div className="rule"><div className="k">as generated</div><div className="v">Artifacts are byte-for-byte what the model wrote. Bugs included.</div></div>
            <div className="rule"><div className="k">tested hard</div><div className="v">C++ compiled with exact flags, servers probed live, <em>everything screenshotted</em>.</div></div>
          </Reveal>
        </div>
      </section>

      <section id="method">
        <div className="wrap">
          <div className="section-head"><h2>How the bench works</h2></div>
          <Reveal className="method">
            <div>
              <p>
                Each task starts as a fresh agent session: <code>opencode run --auto</code> in an empty
                directory, pointed at whichever model currently serves this cluster&apos;s single OpenAI-compatible
                endpoint. The model gets the prompt, a shell, and file tools — the same setup our real coding
                agents use in production. It can compile, run, and test its own work. Whether it bothers to
                is part of what&apos;s being measured.
              </p>
              <p>
                A transparent proxy sits between the harness and vLLM. For DeepSeek it injects{" "}
                <code>reasoning_effort: max</code> — the model has a real thinking dial, so we run it wide
                open. MiniMax&apos;s interleaved thinking is always on and has no dial (we checked: the parameter
                is silently ignored, and the open-weights chat template has no budget variable), so it runs
                at its natural setting. Each model at the best configuration it actually offers.
              </p>
            </div>
            <div>
              <p>
                Scoring is not vibes. The C++ tasks were re-compiled from scratch with the exact commands in
                the prompts and executed on this cluster&apos;s own CPUs. The chat servers were probed with real
                sockets and scripted multi-client conversations. The bug hunt has a sealed eight-bug answer
                key written before either model saw the code. The visual tasks were screenshotted in a real
                browser — which is how we caught a page that renders as garbage despite flawless-looking code.
              </p>
              <p>
                The scores here are <strong>Fable&apos;s</strong> — the Claude agent that operates this cluster,
                ran both legs, and did the forensics. They&apos;re opinionated and they&apos;re hidden until you
                commit your own, because the whole point of publishing the raw artifacts is that{" "}
                <strong>you don&apos;t have to take anyone&apos;s word for it.</strong>
              </p>
            </div>
          </Reveal>
        </div>
      </section>

      <StatsSection />
      <TaskGrid />
      <Standings />
    </main>
  );
}
