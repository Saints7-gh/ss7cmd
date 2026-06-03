***
# ⚡ SS7CMD v1.0.0-TITAN — TITAN Command Framework

**LIGHT speed · TITAN complexity · Zero dependencies**

SS7CMD is a modern, high-performance command processor for SA-MP and open.mp servers. Built from the ground up with **zero-copy parsing**, **O(1) hash table dispatch**, permissions, groups, aliases, cooldowns, typo suggestions, and a complete UI framework — all without a single external plugin.

---

## 🚀 Why SS7CMD?

| Feature | SS7CMD | Other Processors |
| :--- | :--- | :--- |
| **Dispatch speed** | ~18 µs (with 3 params) | 20–50 µs |
| **Permission system** | ✅ Group inheritance | ❌ or basic only |
| **Alias system** | ✅ O(1) hash lookup | ❌ or linear scan |
| **Cooldown & antispam** | ✅ Built-in | Manual implementation |
| **Typo suggestion** | ✅ Levenshtein distance | ❌ |
| **Zero-copy parser** | ✅ Direct cursor | String copying |
| **UI framework** | ✅ Colors, formatting, dialogs | ❌ |
| **Auto-registration** | ✅ Via `ss7cmdlist.txt` | Manual `OnGameModeInit` |
| **Dependencies** | **ZERO** | Often requires `sscanf` plugin |

---

## 📦 Installation

1. Download the latest release from the [releases page](https://github.com/Saints7-gh/ss7cmd/releases)
2. Extract the `include/` folder to your `qawno/include` or `pawno/include` directory
3. Copy `scriptfiles/ss7cmdlist.txt` to your server's `scriptfiles/` folder
4. Add one line to your gamemode:

```pawn
#include <ss7cmd>
```

That's it. **No plugins. No configuration.**

---

## 🎮 Quick Start

```pawn
#include <open.mp>
#include <ss7cmd>

// ── Simple command (no parameters needed) ──────────────────────────────────
cmd:hello(playerid)
{
    ss7_SendSuccess(playerid, "Hello, world!");
    return CMD_OK;
}

// ── Command with parameters (Zero-Copy Parser) ─────────────────────────────
CMD:give(playerid, params[])
{
    p_s; // Start parsing
    new target, amount, reason[64];

    if (!pis_ex(params, target, amount, reason))
        return ss7_SendSyntax(playerid, "/give [player] [amount] [reason]");

    GivePlayerMoney(target, amount);
    ss7_SendSuccess(playerid, "Done!");
    return CMD_OK;
}

// ── Admin command (/kick — level 100 set in ss7cmdlist.txt) ────────────────
CMD:kick(playerid, params[])
{
    p_s;
    new target;

    if (!p_ex(params, target))
        return ss7_SendSyntax(playerid, "/kick [player]");

    Kick(target);
    ss7_SendSuccess(playerid, "Player kicked.");
    return CMD_OK;
}

// ── Heal command (/heal — cooldown 5s set in ss7cmdlist.txt) ──────────────
CMD:heal(playerid, params[])
{
    #pragma unused params
    SetPlayerHealth(playerid, 100.0);
    ss7_SendSuccess(playerid, "You have been healed!");
    return CMD_OK;
}

// ── Metadata and aliases (in OnGameModeInit) ──────────────────────────────
public OnGameModeInit()
{
    CMD_META(heal, "Restore your health to full", "/heal", "General")
    CMD_META(kick, "Kick a player from the server", "/kick [player]", "Admin")
    SS7_ALIAS(hp, heal)     // /hp → /heal
    SS7_ALIAS(k,  kick)     // /k  → /kick
    return 1;
}
```

---

## ⚙️ Command Registration — `ss7cmdlist.txt`

Commands are registered via `scriptfiles/ss7cmdlist.txt` — **no `OnGameModeInit` code needed** for registration. Each line defines one command:

```
# Format: command_name  level  cooldown_ms
# Lines starting with # are comments

# Basic commands (everyone)
hello       0       0
heal        0       5000
stats       0       0
id          0       0

# VIP commands
vipheal     10      10000

# Moderator commands
clearchat   50      0
warn        50      0
mute        50      0

# Admin commands (level 100)
kick        100     0
ban         100     0
setlevel    100     0

# Super Admin
setadmin    200     0
restart     200     0

# Owner
shutdown    255     0
```

> **Columns:** `command_name` (required) · `level` (default: 0) · `cooldown_ms` (default: 0)  
> To reload at runtime without restarting: call `ss7cmdlist_reload()` from any command.

---

## 📚 Core Features

### 🔤 Command Macros

| Macro | Parameters | Description |
| :--- | :--- | :--- |
| `CMD:name(playerid, params[])` | With params | Standard command with parameter parsing |
| `cmd:name(playerid)` | No params | Shorthand for commands that take no arguments |
| `CMD_META(name, desc, usage, cat)` | — | Set description, usage, and category (global scope) |
| `CMD_ALIAS(new, existing)` | — | Compile-time alias (generates a new function) |
| `SS7_ALIAS(alias, target)` | — | Runtime alias via hash table |

---

### ⚙️ Parser Macros

The zero-copy parser extracts parameters **directly without `strmid` or dynamic string allocation** — cursor-based, allocation-free.

| Macro | Declares | Type | Example |
| :--- | :--- | :--- | :--- |
| `p_s` | cursor | Start parsing | `p_s;` |
| `i(params, val)` | `new val` | Integer | — |
| `f(params, val)` | `new Float:val` | Float | — |
| `p(params, val)` | `new val` | Player ID | — |
| `s(params, text)` | `new text[128]` | String | — |
| `i_ex(params, val)` | existing | Integer | `new x; i_ex(params, x);` |
| `f_ex(params, val)` | existing | Float | `new Float:x; f_ex(params, x);` |
| `p_ex(params, val)` | existing | Player | `new t; p_ex(params, t);` |
| `pi_ex(params, t, a)` | existing | Player + Int | — |
| `pis_ex(params, t, a, r)` | existing | Player + Int + String | — |
| `pi(params, t, a)` | `new` | Player + Int | — |
| `pis(params, t, a, r)` | `new` | Player + Int + String | — |

**Parser returns `false` if required arguments are missing** — use with `if (!...)` for automatic syntax validation.

---

### 🛡️ Permission Levels

| Level Name | Constant | Value |
| :--- | :--- | :--- |
| Guest | `SS7_LEVEL_GUEST` | 0 |
| VIP | `SS7_LEVEL_VIP` | 10 |
| Helper | `SS7_LEVEL_HELPER` | 25 |
| Moderator | `SS7_LEVEL_MOD` | 50 |
| Admin | `SS7_LEVEL_ADMIN` | 100 |
| Super Admin | `SS7_LEVEL_SADMIN` | 200 |
| Owner | `SS7_LEVEL_OWNER` | 255 |

Set player level at runtime:
```pawn
ss7_SetPlayerLevel(playerid, SS7_LEVEL_ADMIN);
new level = ss7_GetPlayerLevel(playerid);
```

---

### 👥 Permission Groups

Create custom permission nodes with bitfield inheritance:

```pawn
// Register permission nodes first (returns node index)
new permWarn = ss7_RegisterPerm("warn");
new permMute = ss7_RegisterPerm("mute");
new permKick = ss7_RegisterPerm("kick");
new permBan  = ss7_RegisterPerm("ban");

// Create groups
new gMod   = ss7_CreateGroup("Moderator");
new gAdmin = ss7_CreateGroup("Admin", gMod);  // Admin inherits from Moderator

// Grant permissions to groups (using node indices)
ss7_GroupGrant(gMod,   permWarn);
ss7_GroupGrant(gMod,   permMute);
ss7_GroupGrant(gAdmin, permKick);
ss7_GroupGrant(gAdmin, permBan);

// Assign group to player
ss7_SetPlayerGroup(playerid, gAdmin);

// Check permission (using node index)
if (ss7_PlayerHasPerm(playerid, permBan))
{
    SendClientMessage(playerid, -1, "You can ban players!");
}
```

Note: ss7_RegisterPerm() returns an integer node index. Use this index with ss7_GroupGrant() and ss7_PlayerHasPerm() — not the string name.

```

---

### ⏱️ Cooldowns & Anti-spam

Cooldowns are configured per-command in `ss7cmdlist.txt` (milliseconds):

```
heal    0    5000    # 5 second cooldown
tp      0    3000    # 3 second cooldown
```

Anti-spam is built-in and configurable:

```pawn
// In ss7_core.inc (adjust before including):
#define SS7_ANTISPAM_MS     300   // 300ms between any commands
#define SS7_ENABLE_COOLDOWN 1     // Per-command cooldown
#define SS7_ENABLE_ANTISPAM 1     // Global anti-spam
```

---

### 💡 Typo Suggestions

When a player types an unknown command, SS7CMD automatically suggests the closest match using **Levenshtein distance**:

```
> /heall
Unknown command. Did you mean /heal?
```

Enable/disable:
```pawn
#define SS7_TYPO_SUGGEST 1   // 1 = enabled (default), 0 = disabled
```

---

### 🎨 UI & Formatting Helpers

```pawn
// Client messages
ss7_SendSuccess(playerid, "Done!");
ss7_SendError(playerid, "Something went wrong.");
ss7_SendInfo(playerid, "Information message.");
ss7_SendWarn(playerid, "Warning: something needs attention.");
ss7_SendSyntax(playerid, "/cmd [param1] [param2]");

// Number formatting
new buf[32];
ss7_FormatNumber(1234567, buf);   // "1,234,567"
ss7_FormatMoney(5000, buf);       // "$5,000"
ss7_FormatTime(3665, buf);        // "1h 1m 5s"
ss7_ProgressBar(75.0, buf, 20);   // "[||||||||||||||||    ]"

// Dialogs
ss7_ShowInput(playerid, "Title", "Enter your name:");
ss7_ShowPassword(playerid, "Login", "Enter your password:");
ss7_ShowList(playerid, "Choose", "Option 1\nOption 2\nOption 3");
ss7_ShowMsgBox(playerid, "Notice", "Server restarting in 5 minutes.");
```

---

### 🔁 Runtime Reload

Reload command list without restarting the server:

```pawn
CMD:reloadcmds(playerid, params[])
{
    #pragma unused params
    if (ss7_GetPlayerLevel(playerid) < SS7_LEVEL_OWNER) return 0;
    ss7cmdlist_reload();
    ss7_SendSuccess(playerid, "Command list reloaded.");
    return CMD_OK;
}
```

---

## 🔧 Performance Benchmarks

Tested on Emulated Environment (100,000 Iterations) — SS7CMD v1.0.0

| Operation | Time (µs/iter) |
| :--- | :--- |
| Bitwise permission eval | `~ 1.15 µs` |
| String hash (FNV-1a) | `~ 3.91 µs` |
| Command lookup O(1) | `~ 8.35 µs` |
| Dispatch (Not Found Skip) | `~ 10.10 µs` |
| Zero-copy parse macros | `~ 13.43 µs` |
| Full dispatch (Empty Cmd) | `~ 16.98 µs` |
| Full dispatch (3 parameters) | `~ 25.50 µs` |

---

## 📁 File Structure

```
ss7cmd/
├── include/
│   ├── ss7cmd.inc          ← Master include (only file you need)
│   ├── ss7_core.inc        ← Hash table, dispatch, registration
│   ├── ss7_hook.inc        ← ALS hooks (OnGameModeInit, OnPlayerCommandText, etc.)
│   ├── ss7_parser.inc      ← Zero-copy parameter parser
│   ├── ss7_registry.inc    ← CMD/cmd macros, aliases, metadata
│   ├── ss7_runtime.inc     ← Groups, permissions, level management
│   └── ss7_ui.inc          ← Message helpers, formatting, dialogs
└── scriptfiles/
    └── ss7cmdlist.txt      ← Command registration (name, level, cooldown)
```

---

## 🔗 SS7DB Integration (Future Release)

SS7CMD works seamlessly alongside **SS7DB (TITAN Persistence Engine)** — a companion library for complete server persistence with zero boilerplate.

```pawn
#include <open.mp>
#include <ss7cmd>   
#include <ss7db>    
```

**SS7DB features:**
- 📐 Schema-as-Code with `SS7DB_SCHEMA_BEGIN` / `SS7DB_SCHEMA_END`
- 🔄 Auto-migration with `ALTER TABLE ADD COLUMN` detection
- 🔐 Built-in account system (register dialog, login dialog, 2-step confirm)
- 💾 Hot/Warm/Cold data layer architecture
- 🧹 Zero-config: auto load on connect, auto flush on disconnect
- 🗄️ SQLite with WAL mode for performance

> SS7DB is available as a separate release. See the [SS7DB repository](#) for details.

---

## ⚙️ Configuration Reference

All configuration is done via `#define` **before** `#include <ss7cmd>`:

```pawn
// Limits Example Usage
#define SS7_MAX_COMMANDS    128     // Maximum number of registered commands
#define SS7_MAX_PLAYERS     MAX_PLAYERS
#define SS7_MAX_PERMS       32      // Maximum custom permission nodes
#define SS7_MAX_GROUPS      16      // Maximum permission groups

// Features (1 = enabled, 0 = disabled)
#define SS7_ENABLE_COOLDOWN 1
#define SS7_ENABLE_ANTISPAM 1
#define SS7_TYPO_SUGGEST    1

// Tuning
#define SS7_ANTISPAM_MS     300     // Global anti-spam window (ms)
#define SS7_MAX_CMD_NAME    32      // Max command name length

// Disable auto-load from ss7cmdlist.txt
// #define SS7_NO_AUTOLOAD
```

---

## 📄 License

**MIT License** — free for commercial and personal use.

---

## 🙏 Credits

- **Design & Architecture:** TITAN Framework
- **Core Algorithms:** FNV-1a hashing, Levenshtein distance
- **Compatibility:** SA-MP 0.3.7 · open.mp 1.x

---

<p align="center">Made with ⚡ for the SA-MP / open.mp community.</p>

***
