// =============================================================================
//   ss7_core.inc — TITAN Version
//   Pure Logic Engine: registry, dispatch, permission, group inheritance
// =============================================================================

#if defined _ss7_core_included
    #endinput
#endif
#define _ss7_core_included

// =============================================================================
//  CONFIGURATION
// =============================================================================

#define SS7_MAX_COMMANDS        512
#define SS7_MAX_CMD_NAME        32
#define SS7_HASH_BUCKETS        1024
#define SS7_HASH_MASK           (SS7_HASH_BUCKETS - 1)
#define SS7_MAX_PARAMS          256
#define SS7_MAX_GROUPS          64
#define SS7_MAX_PERMS           64
#define SS7_BIT_CELLS           32
#define SS7_MAX_INHERIT_DEPTH   8
#define SS7_TYPO_MAX_DIST       2
#define SS7_TYPO_COOLDOWN_MS    1500

#define SS7_MAX_ALIASES         256
#define SS7_MAX_ALIAS_NAME      32
#define SS7_ALIAS_SLOT_BASE     SS7_MAX_COMMANDS

#define SS7_CASE_INSENSITIVE    1
#define SS7_ENABLE_COOLDOWN     1
#define SS7_ENABLE_ANTISPAM     1
#define SS7_ANTISPAM_MS         300
#define SS7_TYPO_SUGGEST        1

// =============================================================================
//  DISPATCH RETURN CODES
// =============================================================================

#define SS7_HANDLED             1
#define SS7_OK                  1
#define SS7_NOT_FOUND           0
#define SS7_DENY_PERMISSION    -1
#define SS7_DENY_COOLDOWN      -2
#define SS7_DENY_SPAM          -3
#define SS7_DENY_DISABLED      -4

#define SS7_LEVEL_GUEST         0
#define SS7_LEVEL_VIP           10
#define SS7_LEVEL_HELPER        25
#define SS7_LEVEL_MOD           50
#define SS7_LEVEL_ADMIN         100
#define SS7_LEVEL_SADMIN        200
#define SS7_LEVEL_OWNER         255

#define SS7_INVALID_CMD         -1
#define SS7_INVALID_PERM        -1
#define SS7_INVALID_GROUP       -1

#define CMD_HANDLED             SS7_HANDLED
#define CMD_OK                  SS7_OK
#define CMD_USAGE               -3
#define CMD_DENIED              SS7_DENY_PERMISSION

// =============================================================================
//  DATA STRUCTURES
// =============================================================================

static _ss7_CmdName    [SS7_MAX_COMMANDS][SS7_MAX_CMD_NAME];
static _ss7_CmdHash    [SS7_MAX_COMMANDS];
static _ss7_CmdPerm    [SS7_MAX_COMMANDS];
static _ss7_CmdGroup   [SS7_MAX_COMMANDS];
static _ss7_CmdLevel   [SS7_MAX_COMMANDS];
static _ss7_CmdCooldown[SS7_MAX_COMMANDS];
static _ss7_CmdEnabled [SS7_MAX_COMMANDS];
static _ss7_CmdLen     [SS7_MAX_COMMANDS];
static _ss7_CmdFuncName[SS7_MAX_COMMANDS][SS7_MAX_CMD_NAME + 8];
static _ss7_CmdCount   = 0;

static _ss7_BucketHead [SS7_HASH_BUCKETS] = {-1, ...};
static _ss7_ChainNext  [SS7_MAX_COMMANDS] = {-1, ...};

static _ss7p_GrpName   [SS7_MAX_GROUPS][SS7_MAX_CMD_NAME];
static _ss7p_GrpParent [SS7_MAX_GROUPS];
static _ss7p_GrpPerms  [SS7_MAX_GROUPS][SS7_BIT_CELLS];
static _ss7p_GrpCount  = 0;

static _ss7_PlrGroup   [MAX_PLAYERS];
static _ss7_PlrPerms   [MAX_PLAYERS][SS7_BIT_CELLS];
static _ss7_PlayerLevel[MAX_PLAYERS];
static _ss7_SpamTick   [MAX_PLAYERS];
static _ss7_TypoTick   [MAX_PLAYERS];

#if SS7_ENABLE_COOLDOWN
    static _ss7_LastUsed[MAX_PLAYERS][SS7_MAX_COMMANDS];
#endif

static _ss7_PermName   [SS7_MAX_PERMS][32];
static _ss7_PermCount  = 0;

static _ss7_AliasName  [SS7_MAX_ALIASES][SS7_MAX_ALIAS_NAME];
static _ss7_AliasHash  [SS7_MAX_ALIASES];
static _ss7_AliasTarget[SS7_MAX_ALIASES];
static _ss7_AliasNext  [SS7_MAX_ALIASES] = {-1, ...};
static _ss7_AliasCount = 0;

static _ss7_CmdDesc    [SS7_MAX_COMMANDS][64];
static _ss7_CmdUsage   [SS7_MAX_COMMANDS][64];
static _ss7_CmdCategory[SS7_MAX_COMMANDS][32];

// =============================================================================
//  INTERNAL HELPERS
// =============================================================================

static stock _ss7_strncpy(dest[], const source[], maxlen)
{
    new i = 0;
    while (source[i] != '\0' && i < maxlen - 1)
    {
        dest[i] = source[i];
        i++;
    }
    dest[i] = '\0';
}

// =============================================================================
//  FNV-1a HASH ENGINE
// =============================================================================

static stock _ss7_FNV1a(const str[])
{
    new hash = 0x811C9DC5;
    new i = 0, ch;
    while ((ch = str[i++]) != '\0')
    {
        #if SS7_CASE_INSENSITIVE
            if (ch >= 'A' && ch <= 'Z') ch += 32;
        #endif
        hash ^= ch;
        hash *= 0x01000193;
    }
    return hash;
}

// =============================================================================
//  HASH TABLE OPERATIONS
// =============================================================================

static stock _ss7_InsertHash(idx)
{
    new bucket = _ss7_CmdHash[idx] & SS7_HASH_MASK;
    _ss7_ChainNext[idx] = _ss7_BucketHead[bucket];
    _ss7_BucketHead[bucket] = idx;
}

static stock _ss7_FindCmdByHash(hash, const name[])
{
    new bucket = hash & SS7_HASH_MASK;
    new idx = _ss7_BucketHead[bucket];

    while (idx != -1)
    {
        if (_ss7_CmdHash[idx] == hash)
        {
            #if SS7_CASE_INSENSITIVE
                if (!strcmp(_ss7_CmdName[idx], name, true)) return idx;
            #else
                if (!strcmp(_ss7_CmdName[idx], name, false)) return idx;
            #endif
        }
        idx = _ss7_ChainNext[idx];
    }

    for (new a = 0; a < _ss7_AliasCount; a++)
    {
        if (_ss7_AliasHash[a] == hash)
        {
            #if SS7_CASE_INSENSITIVE
                if (!strcmp(_ss7_AliasName[a], name, true)) return _ss7_AliasTarget[a];
            #else
                if (!strcmp(_ss7_AliasName[a], name, false)) return _ss7_AliasTarget[a];
            #endif
        }
    }
    return SS7_INVALID_CMD;
}

static stock _ss7_FindCmd(const name[])
{
    return _ss7_FindCmdByHash(_ss7_FNV1a(name), name);
}

// =============================================================================
//  COMMAND REGISTRATION
// =============================================================================

stock ss7_GetCmdIndex(const cmdname[])
{
    return _ss7_FindCmd(cmdname);
}

stock ss7_GetCmdLevel(const cmdname[])
{
    new idx = _ss7_FindCmd(cmdname);
    if (idx == SS7_INVALID_CMD) return -1;
    return _ss7_CmdLevel[idx];
}

stock ss7_RegisterCmd(const name[], level = SS7_LEVEL_GUEST)
{
    if (_ss7_CmdCount >= SS7_MAX_COMMANDS)
    {
        printf("[SS7-CORE] ERROR: Command table full — cannot register: /%s", name);
        return -1;
    }

    new idx = _ss7_CmdCount;
    new hash = _ss7_FNV1a(name);

    if (_ss7_FindCmdByHash(hash, name) != SS7_INVALID_CMD) return -1;

    _ss7_strncpy(_ss7_CmdName[idx], name, SS7_MAX_CMD_NAME);
    _ss7_CmdHash[idx] = hash;
    _ss7_CmdPerm[idx] = -1;
    _ss7_CmdGroup[idx] = -1;
    _ss7_CmdLevel[idx] = level;
    _ss7_CmdCooldown[idx] = 0;
    _ss7_CmdEnabled[idx] = 1;
    _ss7_CmdLen[idx] = strlen(name);
    _ss7_CmdFuncName[idx][0] = '\0';
    strcat(_ss7_CmdFuncName[idx], "ss7_Cmd_", SS7_MAX_CMD_NAME + 8);
    strcat(_ss7_CmdFuncName[idx], name, SS7_MAX_CMD_NAME + 8);

    _ss7_InsertHash(idx);
    _ss7_CmdCount++;
    return idx;
}

stock ss7_RegisterCmdEx(const name[], level = SS7_LEVEL_GUEST, group = -1, cooldown_ms = 0)
{
    new idx = ss7_RegisterCmd(name, level);
    if (idx == -1) return -1;

    if (group != -1) _ss7_CmdGroup[idx] = group;
    if (cooldown_ms > 0) _ss7_CmdCooldown[idx] = cooldown_ms;

    return idx;
}

// =============================================================================
//  PERMISSION GROUP
// =============================================================================

stock ss7_CreateGroup(const name[], parentIdx = -1)
{
    if (_ss7p_GrpCount >= SS7_MAX_GROUPS)
    {
        print("[SS7-CORE] ERROR: Group table full.");
        return -1;
    }
    new gid = _ss7p_GrpCount;
    _ss7_strncpy(_ss7p_GrpName[gid], name, SS7_MAX_CMD_NAME);
    _ss7p_GrpParent[gid] = parentIdx;
    for (new i = 0; i < SS7_BIT_CELLS; i++) _ss7p_GrpPerms[gid][i] = 0;
    _ss7p_GrpCount++;
    return gid;
}

stock ss7_GroupGrant(groupIdx, node)
{
    if (groupIdx < 0 || groupIdx >= _ss7p_GrpCount) return;
    _ss7p_GrpPerms[groupIdx][node >>> 5] |= (1 << (node & 31));
}

stock ss7_GroupRevoke(groupIdx, node)
{
    if (groupIdx < 0 || groupIdx >= _ss7p_GrpCount) return;
    _ss7p_GrpPerms[groupIdx][node >>> 5] &= ~(1 << (node & 31));
}

stock bool:ss7_GroupHasDirect(groupIdx, node)
{
    if (groupIdx < 0 || groupIdx >= _ss7p_GrpCount) return false;
    return (_ss7p_GrpPerms[groupIdx][node >>> 5] & (1 << (node & 31))) != 0;
}

// =============================================================================
//  PERMISSION RESOLVER
// =============================================================================

static stock _ss7_RebuildPlayerPerms(playerid)
{
    for (new i = 0; i < SS7_BIT_CELLS; i++) _ss7_PlrPerms[playerid][i] = 0;

    new current = _ss7_PlrGroup[playerid];
    new depth = 0;

    while (current != -1 && depth < SS7_MAX_INHERIT_DEPTH)
    {
        for (new i = 0; i < SS7_BIT_CELLS; i++)
            _ss7_PlrPerms[playerid][i] |= _ss7p_GrpPerms[current][i];
        current = _ss7p_GrpParent[current];
        depth++;
    }
}

// =============================================================================
//  PLAYER STATE API
// =============================================================================

stock ss7_GetPlayerLevel(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;
    return _ss7_PlayerLevel[playerid];
}

stock ss7_SetPlayerLevel(playerid, level)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return;
    _ss7_PlayerLevel[playerid] = level;
}

stock ss7_SetPlayerGroup(playerid, groupIdx)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return;
    _ss7_PlrGroup[playerid] = groupIdx;
    _ss7_RebuildPlayerPerms(playerid);
}

stock ss7_GetPlayerGroup(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return -1;
    return _ss7_PlrGroup[playerid];
}

stock bool:ss7_PlayerHasPerm(playerid, node)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    return (_ss7_PlrPerms[playerid][node >>> 5] & (1 << (node & 31))) != 0;
}

// =============================================================================
//  PLAYER RESET
// =============================================================================

stock _ss7_Internal_ResetPlayer(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 0;

    _ss7_PlrGroup  [playerid] = -1;
    _ss7_SpamTick  [playerid] = 0;
    _ss7_TypoTick  [playerid] = 0;
    _ss7_PlayerLevel[playerid] = SS7_LEVEL_GUEST;

    for (new i = 0; i < SS7_BIT_CELLS; i++) _ss7_PlrPerms[playerid][i] = 0;

    #if SS7_ENABLE_COOLDOWN
        for (new c = 0; c < _ss7_CmdCount; c++) _ss7_LastUsed[playerid][c] = 0;
    #endif

    return 1;
}

// =============================================================================
//  INPUT PARSER
// =============================================================================

static stock _ss7_ParseInput(const input[], cmdname[], params[], &cmdHash)
{
    if (input[0] != '/') { cmdHash = 0; return 0; }

    new hash = 0x811C9DC5;
    new i = 1, j = 0, c;

    while ((c = input[i]) != '\0' && c != ' ' && j < SS7_MAX_CMD_NAME - 1)
    {
        #if SS7_CASE_INSENSITIVE
            if (c >= 'A' && c <= 'Z') c += 32;
        #endif
        cmdname[j++] = c;
        hash ^= c;
        hash *= 0x01000193;
        i++;
    }
    cmdname[j] = '\0';
    if (j == 0) { cmdHash = 0; return 0; }
    cmdHash = hash;

    while (input[i] == ' ') i++;

    j = 0;
    while ((c = input[i]) != '\0' && j < SS7_MAX_PARAMS - 1)
    {
        params[j++] = c;
        i++;
    }
    params[j] = '\0';
    return 1;
}

// =============================================================================
//  MAIN DISPATCH
// =============================================================================

stock ss7_Dispatch(playerid, const cmdtext[])
{
    new _name[SS7_MAX_CMD_NAME];
    new _params[SS7_MAX_PARAMS];
    new _h;

    if (!_ss7_ParseInput(cmdtext, _name, _params, _h)) return SS7_NOT_FOUND;

    new idx = _ss7_FindCmdByHash(_h, _name);

    if (idx == SS7_INVALID_CMD || !_ss7_CmdEnabled[idx]) return SS7_NOT_FOUND;

    new _now = GetTickCount();

    if (_ss7_PlayerLevel[playerid] < _ss7_CmdLevel[idx])
    {
        SendClientMessage(playerid, 0xFF3333FF, "You don't have permission.");
        return SS7_HANDLED;
    }

    #if SS7_ENABLE_COOLDOWN
        if (_ss7_CmdCooldown[idx] > 0)
        {
            new elapsed = _now - _ss7_LastUsed[playerid][idx];
            if (_ss7_LastUsed[playerid][idx] != 0 && elapsed < _ss7_CmdCooldown[idx])
            {
                new remaining = (_ss7_CmdCooldown[idx] - elapsed) / 1000 + 1;
                new msg[96];
                format(msg, sizeof(msg), " Please wait %d second(s).", remaining);
                SendClientMessage(playerid, 0xFF9900FF, msg);
                return SS7_HANDLED;
            }
            _ss7_LastUsed[playerid][idx] = _now;
        }
    #endif

    #if SS7_ENABLE_ANTISPAM
        new spam_elapsed = _now - _ss7_SpamTick[playerid];
        if (_ss7_SpamTick[playerid] != 0 && spam_elapsed < SS7_ANTISPAM_MS)
        {
            SendClientMessage(playerid, 0xFF9900FF, " Too fast. Slow down.");
            return SS7_HANDLED;
        }
        _ss7_SpamTick[playerid] = _now;
    #endif

    return CallLocalFunction(_ss7_CmdFuncName[idx], "is", playerid, _params);
}

// =============================================================================
//  TYPO SUGGESTION
// =============================================================================

#if SS7_TYPO_SUGGEST

static stock _ss7_Levenshtein(const a[], const b[], maxDist)
{
    new la = strlen(a), lb = strlen(b);
    new diff = la - lb; if (diff < 0) diff = -diff;
    if (diff > maxDist) return maxDist + 1;

    new prev[SS7_MAX_CMD_NAME + 1], curr[SS7_MAX_CMD_NAME + 1];

    for (new j = 0; j <= lb; j++) prev[j] = j;

    for (new i = 1; i <= la; i++)
    {
        curr[0] = i;
        new rowMin = i;

        for (new j = 1; j <= lb; j++)
        {
            new cost = (a[i-1] == b[j-1]) ? 0 : 1;
            new del = prev[j] + 1;
            new ins = curr[j-1] + 1;
            new sub = prev[j-1] + cost;

            curr[j] = del;
            if (ins < curr[j]) curr[j] = ins;
            if (sub < curr[j]) curr[j] = sub;

            if (curr[j] < rowMin) rowMin = curr[j];
        }

        if (rowMin > maxDist) return maxDist + 1;

        for (new j = 0; j <= lb; j++) prev[j] = curr[j];
    }
    return prev[lb];
}

stock ss7_GetSuggestion(playerid, const typed[], dest[], destLen)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return SS7_INVALID_CMD;

    new now = GetTickCount();
    if ((now - _ss7_TypoTick[playerid]) < SS7_TYPO_COOLDOWN_MS)
    {
        dest[0] = '\0';
        return SS7_INVALID_CMD;
    }
    _ss7_TypoTick[playerid] = now;

    new bestDist = SS7_TYPO_MAX_DIST + 1, bestIdx = -1;

    for (new i = 0; i < _ss7_CmdCount; i++)
    {
        new d = _ss7_Levenshtein(typed, _ss7_CmdName[i], SS7_TYPO_MAX_DIST);
        if (d < bestDist)
        {
            bestDist = d;
            bestIdx = i;
            if (d == 0) break;
        }
    }

    if (bestIdx != -1 && bestDist <= SS7_TYPO_MAX_DIST)
    {
        _ss7_strncpy(dest, _ss7_CmdName[bestIdx], destLen);
        return bestIdx;
    }
    dest[0] = '\0';
    return SS7_INVALID_CMD;
}

#endif

// =============================================================================
//  REFLECTION API
// =============================================================================

stock ss7_ForEachCmd(const callback[])
{
    for (new i = 0; i < _ss7_CmdCount; i++)
        CallLocalFunction(callback, "issii", i, _ss7_CmdName[i], "", _ss7_CmdLevel[i], _ss7_CmdGroup[i]);
}

// =============================================================================
//  INITIALIZATION
// =============================================================================

stock ss7_Init()
{
    for (new i = 0; i < SS7_HASH_BUCKETS; i++) _ss7_BucketHead[i] = -1;
    for (new i = 0; i < SS7_MAX_COMMANDS; i++) _ss7_ChainNext[i] = -1;
    for (new i = 0; i < SS7_MAX_ALIASES; i++) _ss7_AliasNext[i] = -1;

    _ss7_CmdCount = 0;
    _ss7p_GrpCount = 0;
    _ss7_PermCount = 0;
    _ss7_AliasCount = 0;

    print("═══════════════════════════════════════════════════════════");
    print(" [SS7-CORE] TITAN v1.0.0 — LIGHT speed + TITAN complexity");
    printf(" [SS7-CORE] Hash buckets: %d | Max commands: %d", SS7_HASH_BUCKETS, SS7_MAX_COMMANDS);
    printf(" [SS7-CORE] Group slots: %d | Perm nodes: %d", SS7_MAX_GROUPS, SS7_MAX_PERMS);
    print("═══════════════════════════════════════════════════════════");
    return 1;
}

// =============================================================================
//  PERMISSION NODES API
// =============================================================================

stock ss7_RegisterPerm(const name[])
{
    if (_ss7_PermCount >= SS7_MAX_PERMS)
    {
        printf("[SS7-CORE] ERROR: Permission table full — cannot register: %s", name);
        return -1;
    }

    for (new i = 0; i < _ss7_PermCount; i++)
    {
        if (!strcmp(_ss7_PermName[i], name, true)) return i;
    }

    _ss7_strncpy(_ss7_PermName[_ss7_PermCount], name, 32);
    _ss7_PermCount++;
    return _ss7_PermCount - 1;
}

stock ss7_FindPerm(const name[])
{
    for (new i = 0; i < _ss7_PermCount; i++)
    {
        if (!strcmp(_ss7_PermName[i], name, true)) return i;
    }
    return -1;
}

stock ss7_GetPermName(permid, dest[], destLen)
{
    if (permid < 0 || permid >= _ss7_PermCount)
    {
        dest[0] = '\0';
        return 0;
    }
    _ss7_strncpy(dest, _ss7_PermName[permid], destLen);
    return 1;
}

stock ss7_GetPermCount()
{
    return _ss7_PermCount;
}

// =============================================================================
//  COOLDOWN API
// =============================================================================

stock ss7_SetCmdCooldown(const cmdname[], cooldown_ms)
{
    new idx = _ss7_FindCmd(cmdname);
    if (idx == SS7_INVALID_CMD) return 0;
    _ss7_CmdCooldown[idx] = cooldown_ms;
    return 1;
}

stock ss7_GetCmdCooldown(const cmdname[])
{
    new idx = _ss7_FindCmd(cmdname);
    if (idx == SS7_INVALID_CMD) return -1;
    return _ss7_CmdCooldown[idx];
}

// =============================================================================
//  METADATA API
// =============================================================================

stock ss7_SetMeta(cmdIdx, const description[], const usage[], const category[])
{
    if (cmdIdx < 0 || cmdIdx >= _ss7_CmdCount) return 0;

    _ss7_strncpy(_ss7_CmdDesc[cmdIdx], description, 64);
    _ss7_strncpy(_ss7_CmdUsage[cmdIdx], usage, 64);
    _ss7_strncpy(_ss7_CmdCategory[cmdIdx], category, 32);
    return 1;
}

stock ss7_GetCmdDescription(cmdIdx, dest[], destLen)
{
    if (cmdIdx < 0 || cmdIdx >= _ss7_CmdCount)
    {
        dest[0] = '\0';
        return 0;
    }
    _ss7_strncpy(dest, _ss7_CmdDesc[cmdIdx], destLen);
    return 1;
}

stock ss7_GetCmdUsage(cmdIdx, dest[], destLen)
{
    if (cmdIdx < 0 || cmdIdx >= _ss7_CmdCount)
    {
        dest[0] = '\0';
        return 0;
    }
    _ss7_strncpy(dest, _ss7_CmdUsage[cmdIdx], destLen);
    return 1;
}

stock ss7_GetCmdCategory(cmdIdx, dest[], destLen)
{
    if (cmdIdx < 0 || cmdIdx >= _ss7_CmdCount)
    {
        dest[0] = '\0';
        return 0;
    }
    _ss7_strncpy(dest, _ss7_CmdCategory[cmdIdx], destLen);
    return 1;
}

// =============================================================================
//  ALIAS API
// =============================================================================

stock ss7_AddAlias(cmdIdx, const aliasName[])
{
    if (cmdIdx < 0 || cmdIdx >= _ss7_CmdCount) return 0;
    if (_ss7_AliasCount >= SS7_MAX_ALIASES) return 0;

    new ahash = _ss7_FNV1a(aliasName);
    for (new i = 0; i < _ss7_AliasCount; i++)
    {
        if (_ss7_AliasHash[i] == ahash && !strcmp(_ss7_AliasName[i], aliasName, true))
            return 0;
    }

    new a = _ss7_AliasCount;
    _ss7_strncpy(_ss7_AliasName[a], aliasName, SS7_MAX_ALIAS_NAME);
    _ss7_AliasHash[a] = ahash;
    _ss7_AliasTarget[a] = cmdIdx;
    _ss7_AliasCount++;
    return 1;
}

stock ss7_FindAlias(const aliasName[])
{
    new ahash = _ss7_FNV1a(aliasName);
    for (new i = 0; i < _ss7_AliasCount; i++)
    {
        if (_ss7_AliasHash[i] == ahash)
        {
            #if SS7_CASE_INSENSITIVE
                if (!strcmp(_ss7_AliasName[i], aliasName, true)) return _ss7_AliasTarget[i];
            #else
                if (!strcmp(_ss7_AliasName[i], aliasName, false)) return _ss7_AliasTarget[i];
            #endif
        }
    }
    return SS7_INVALID_CMD;
}

stock ss7_GetAliasCount()
{
    return _ss7_AliasCount;
}

// =============================================================================
//  BENCHMARK API
// =============================================================================

stock ss7_HashString(const str[])
{
    return _ss7_FNV1a(str);
}

stock ss7_FindCmdByHashPublic(hash, const name[])
{
    return _ss7_FindCmdByHash(hash, name);
}

// =============================================================================
//  PUBLIC API — GETTERS
// =============================================================================

stock ss7_GetCmdCount()
{
    return _ss7_CmdCount;
}

// =============================================================================
//  FILE-BASED COMMAND REGISTRATION
// =============================================================================

//static _ss7_cmdlist_loaded = 0;

stock ss7cmdlist_reload()
{
    new File:f = fopen("ss7cmdlist.txt", io_read);
    if (!f)
    {
        printf("[SS7-CORE] WARNING: Cannot open ss7cmdlist.txt from scriptfiles/");
        return 0;
    }
    
    new line[96];
    new cmd[SS7_MAX_CMD_NAME];
    new level;
    new count = 0;
    new errors = 0;
    new line_num = 0;
    
    while (fread(f, line, sizeof(line)))
    {
        line_num++;
        
        new len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        
        if (len == 0) continue;
        
        if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) continue;
        
        new pos = 0;
        new cmdPos = 0;
        
        while (pos < len && line[pos] != ' ' && line[pos] != '\t' && cmdPos < SS7_MAX_CMD_NAME - 1)
            cmd[cmdPos++] = line[pos++];
        cmd[cmdPos] = '\0';
        
        while (pos < len && (line[pos] == ' ' || line[pos] == '\t')) pos++;
        
        if (pos < len)
        {
            level = strval(line[pos]);
        }
        else
        {
            level = SS7_LEVEL_GUEST;
        }
        
        while (pos < len && (line[pos] == ' ' || line[pos] == '\t')) pos++;
        new cooldown_ms = 0;
        if (pos < len && line[pos] >= '0' && line[pos] <= '9')
        {
            cooldown_ms = 0;
            while (pos < len && line[pos] >= '0' && line[pos] <= '9')
            {
                cooldown_ms = cooldown_ms * 10 + (line[pos] - '0');
                pos++;
            }
        }

        // Register command
        new reg_idx = ss7_RegisterCmdEx(cmd, level, -1, cooldown_ms);
        if (reg_idx != -1)
        {
            count++;
        }
        else
        {
            printf("[SS7-CORE] ERROR: Failed to register '%s' at line %d", cmd, line_num);
            errors++;
        }
    }
    
    fclose(f);
    
    //_ss7_cmdlist_loaded = 1;
    printf("[SS7-CORE] Loaded %d commands from ss7cmdlist.txt (%d errors)", count, errors);
    return 1;
}

stock ss7cmdlist()
{
    return ss7cmdlist_reload();
}

// =============================================================================
//  END OF ss7_core.inc
// =============================================================================