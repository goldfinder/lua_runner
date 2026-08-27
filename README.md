# lua_runner

A minimal C-hosted runtime engine for executing modular Lua applications and game logic without external dependencies beyond the compiler binary.

> **Note:** Initial framework logic and C host bindings were generated with AI assistance.

---

## 🏗️ Architecture & File Stack

```text
.
├── Main.c              # Primary C engine source
├── Main.lua            # Primary engine entry script
└── Helpers/
    └── Ticker.lua      # Dynamic module scanner and on_tick manager
