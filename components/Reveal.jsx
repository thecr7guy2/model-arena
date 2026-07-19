export default function Reveal({ children, delay = 0, className = "", style, ...rest }) {
  return (
    <div
      className={`reveal-entry ${className}`.trim()}
      style={{ ...style, "--reveal-delay": `${delay}s` }}
      {...rest}
    >
      {children}
    </div>
  );
}
