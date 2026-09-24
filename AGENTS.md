# Agent Instructions — Unreal Hands Train Sample

An Unreal sample demonstrating hand tracking driving physics interactions — near and distant grabs on physics objects in a train-themed scene on Quest.

## Source-of-truth files (read these first, do not duplicate their contents in this file)

For setup, build steps, SDK versions, and project layout, read:

- `README.md` — official setup, both Epic Launcher and Meta-fork build paths, changelog, known issues
- `HandsTrainSample.uproject` — engine version association and enabled plugins
- `Config/` — `DefaultEngine.ini`, `DefaultGame.ini`, Android platform settings
- `Source/` — C++ module sources
- `Content/` — blueprints, hand-tracking interaction assets (including `LevelBlueprint`)
- `LICENSE` — license terms

## Quest / Horizon-specific notes

- As of the April 2025 update the project uses **Epic's OpenXR** hand tracking, not the legacy Oculus XR plugin path — don't "fix" it by reverting to MetaXR hand tracking.
- On Quest 2 specifically, GPU occlusion queries cause severe FPS drops. The documented fixes are (a) switch to the Meta fork of Unreal (UE5.5.4+) and use CPU occlusion, or (b) add `r.AllowOcclusionQueries 0` via `ExecuteConsoleCommand` (see `LevelBlueprint` for the pattern). Don't propose alternatives before trying these.
- Two build paths: Epic Launcher UE5 + MetaXR plugin (fastest), or building the Meta fork of Unreal Engine from source.
- Git LFS is used by this repo — run `git lfs install` before cloning.

# Meta Quest tooling

This is a Meta Quest / Horizon OS sample. The bespoke intro above is the source of truth for what this project is and how it's built — use it (and the files it points at) instead of restating facts from memory.

When the user asks anything about Quest device behavior, build / deploy / debug / capture flows, on-device performance, or Horizon OS APIs, reach for these tools instead of generic Unreal answers:

- **`hzdb`** — Quest-aware ADB wrapper (device list, install / launch / stop, logs, screenshots, Perfetto traces, on-device docs search). Already wired up as an MCP server via `.mcp.json`, `.vscode/mcp.json`, and `.cursor/mcp.json`. Also runnable directly: `npx -y @meta-quest/hzdb <subcommand>`.
- **Meta Quest Agentic Tools** — the full skill set, including Unreal-specific skills: <https://github.com/meta-quest/agentic-tools>. Install per your client (Claude Code: `/plugin install meta-vr@meta-quest`; Gemini CLI: `gemini extensions install https://github.com/meta-quest/agentic-tools`; Cursor / VS Code: install the **Meta Horizon** extension from the Marketplace).

A few behavior expectations:

- **Read this repo's files first.** Before answering anything project-specific, read `README.md` and whichever source-of-truth files the intro above points at. Don't restate their contents in chat — quote or link instead.
- **Use `hzdb` for device-side work.** Anything that touches an attached Quest (install, launch, logs, screenshot, capture, manifest inspection) goes through `hzdb`, not raw `adb`.
- **Check live Horizon OS docs before answering API questions.** `hzdb docs search "..."` queries the live docs; training data on Horizon OS APIs goes stale fast.
- **Don't fabricate SDK / engine versions.** If a version isn't visible in this repo's files, say so rather than guessing.
