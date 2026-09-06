# OpenGold contributor guidance

- Use RAII for every owned resource in native code.
- Do not represent ownership with raw pointers. Prefer value types and standard
  RAII containers; when pointer semantics are necessary, use `std::unique_ptr`
  by default and `std::shared_ptr` only for genuine shared ownership.
- After updating this repository, commit your changes and push the current branch
  to `origin`, unless the user explicitly asks otherwise. Include only your own
  changes and complete relevant checks before committing.

## Scope and review boundaries

- Answer questions and provide suggestions without treating them as
  authorization to make changes.
- A design request authorizes a written proposal, diagrams, and static
  layout sketches. It does not authorize coding.
- Do not create prototypes, applications, scripts, or launchers for a
  design request.
- When a design is requested for review, deliver it and stop for
  comments. Wait for explicit authorization before implementation.
- Commit/push instructions govern authorized changes; they do not
  expand the scope of a task.

## Technology decisions

- Read docs/TECH.md and the relevant requirements before selecting
  an implementation approach.
- Follow OpenGold's established Godot 4.x, C++20, and GDExtension
  architecture and documented language boundaries.
- Reuse existing components, build tools, and launch conventions.
- Do not introduce another UI stack, runtime, or framework without
  explicit user approval, including for prototypes and review tools.
- Skills and available tools must serve the project's requirements.
  Their availability does not authorize a technology substitution.
- Report specific blockers instead of silently changing technologies.
- Before committing, verify scope and architecture compliance as
  well as functional correctness.

## Time and communication

- Keep responses concise and investigation proportional to the task.
- Do not perform speculative or unrelated work.
- Provide commands appropriate to the user's actual shell.
