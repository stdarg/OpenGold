# SRD implementation workflow

Use [the handoff](SRD-HANDOFF.md) as the entry point, [the issue index](https://github.com/stdarg/OpenGold/issues/186)
as the backlog, and [coverage](SRD-COVERAGE.md) as completion evidence. Keep the
[full implementation plan](SRD-IMPLEMENTATION.md), all twelve classes and later
level/multiclass milestones intact. Efficiency changes execution, not acceptance.

## One issue cycle

1. Read the handoff, `git status --short`, and the selected issue's current body
   and relevant comments. Read `TECH.md` and the applicable feature requirements
   before implementation; reuse the current session's reading. Do not reload the
   entire backlog or audit unless selecting a new milestone or reconciling scope.
2. State a short completion checklist from the issue: source rule, player path,
   affected persistence, independent expected results and relevant checks. Finish
   this issue before opening unrelated work. If a real prerequisite blocks it,
   record the dependency and move to that prerequisite within authorized scope.
3. Use `rg` to locate symbols, then bounded reads. Batch independent reads; keep
   edits and dependent checks sequential. Keep full build logs in ignored local
   storage and return summaries or failure excerpts. Avoid repeatedly polling
   unchanged processes or expanding successful output into the conversation.
4. Gather related UI questions in one numbered request before dependent coding.
   Reuse recorded approvals; ask only about material new choices. Record answers
   in the handoff. A reply to a pending question does not resume a paused goal.
5. Implement and run the focused checks below. Broaden once for shared rules or
   persistence changes. Repeat only affected checks after fixes. Passing narrow
   checks cannot establish whole-feature completion; inspect the actual player
   path and migration behavior required by the issue.
6. Review scope, architecture and the diff, update coverage/support documentation,
   commit only owned changes, push, then close the issue only if all acceptance
   criteria are met. Report the result and remaining dependencies briefly.
7. Replace stale handoff entries with current state. Keep it roughly one page:
   active/next issue, key decisions, relevant paths, verification, pending questions
   and live process handles. Historical detail belongs in commits and issue links.

## Verification selection

These Bash examples use the existing local `build/mac-check` configuration.
Build changed test targets before running them; `ctest` alone can run stale binaries.
Discover names with `ctest --test-dir build/mac-check -N` when needed.

| Change | Minimum relevant evidence |
| --- | --- |
| Documentation/configuration | Diff/links and configuration parsing; no game rebuild. |
| Local rule or grant | Feature test plus affected grant/access/advancement tests. |
| Combat budgets, effects or movement | Feature plus rules/grid/effects tests as affected; shared changes also get native regression suite. |
| Serialization or campaign handoff | Feature, save and campaign continuation tests; real prior-writer fixtures and native regression suite. |
| Controls or presentation | Affected Godot runtime tests and neighboring shared input/layout checks; rendered English/Spanish at 1120×800 and 1920×1080. |

Example focused build and test (replace names for the selected issue):

```bash
cmake --build build/mac-check --target opengold_action_surge_tests -j6
ctest --test-dir build/mac-check --output-on-failure -R '^opengold_action_surge_tests$'
```

For game changes, prepare once after the last native/scene/localization changes:

```bash
cmake --build build/mac-check --target opengoldbox_test_project -j6
ctest --test-dir build/mac-check --output-on-failure -R '^opengold_godot_surge$' --fixture-exclude-setup godot_project
```

Fixture exclusion is valid only after successful preparation of the current
changes. CTest still runs the selected test's native fixture prerequisites.
Run Godot tests serially: internal test checkpoints share a user-data path.
Do not build concurrently in the same build directory. Avoid reconfiguring an
unchanged CMake setup; it can regenerate the Godot bindings unnecessarily.
Use `python3 tools/localization.py --check` after message changes.

For shared rules/persistence changes, build the affected native targets and run:

```bash
ctest --test-dir build/mac-check --output-on-failure -E '^opengold_godot_' -j6
```

See [testing](TESTING.md) for new-machine setup, sanitizers and fuzzing; use those
when warranted by the change, not as a repeated gate for documentation or UI text.

## Reasoning and fresh tasks

`.codex/config.toml` selects `low` for routine project work without changing the
model or global preferences. Use `high` for rule interactions, state machines,
save migrations and difficult debugging. Explicit task settings can override
the project default. Existing tasks may retain their selected setting; wording
in a prompt does not dynamically switch reasoning effort.

In the desktop app, select effort when starting the task. For a CLI task:

```bash
codex -C /Users/edmond/src/OpenGold -c 'model_reasoning_effort="high"'
```

This follows the [official configuration precedence and override guidance](https://learn.chatgpt.com/docs/config-file/config-advanced).
Do not change models or enable parallel agents as an incidental optimization.

At a completed major milestone, use a fresh task when user-authorized. Pass only
the next issue/milestone, repository, handoff link, constraints and done criteria;
do not fork the full conversation. Preserve the overarching goal and its status.
Only one task should own implementation at a time, and pausing must not launch
replacement work. A fresh task does not inherit an active goal automatically.

Suggested starting message after explicit resumption:

> Read AGENTS.md and docs/SRD-HANDOFF.md. Implement the named next milestone using
> docs/SRD-WORKFLOW.md, preserving the full SRD scope and recorded approvals.
> Verify current repository/issue state, complete the issue's acceptance checks,
> commit and push owned changes, and update the handoff and coverage ledger.
