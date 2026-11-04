# AI Agent Instructions & Memory Policy

## 🚨 CRITICAL ARCHITECTURE RULE (READ FIRST)
- **Do not rely on your native platform memory or chat history.**
- This project utilizes a local Git-managed Memory Bank located in the `.agent/` folder to persist knowledge, skills, and progress across different AI tools.
- You must always read from and write to the files in `.agent/` to update your context.

## 💾 MEMORY READ/WRITE SYSTEM PROTOCOL
Before writing any code or answering any prompt, you MUST inspect the files inside the `.agent/` directory:
1. `.agent/active-task.md` — Current active goals, bugs, and immediate next steps.
2. `.agent/project-context.md` — Tech stack, architecture decisions, and learned lessons.
3. `.agent/skills.md` — Custom scripts, commands, and rules you have figured out.

### Your Mandate:
- **Updating Knowledge:** If you discover something new about this codebase, fix a critical bug, or change an architectural decision, you MUST immediately modify the relevant file in the `.agent/` directory.
- **Session Hand-off:** At the end of every task, or if the user asks for a hand-off summary, overwrite `.agent/active-task.md` with an updated layout of what was completed and what remains.
