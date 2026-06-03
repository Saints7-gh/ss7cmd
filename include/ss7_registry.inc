// =============================================================================
//   ss7_registry.inc — Registration API & CMD Macros
//   Module: Registry
//   Description: Command declaration, registration, alias, and metadata macros.
// =============================================================================

#if defined _ss7_registry_included
    #endinput
#endif
#define _ss7_registry_included

#if !defined _ss7_core_included
    #error "[ss7reg] ss7_core.inc must be included before ss7_registry.inc"
#endif

// -----------------------------------------------------------------------------
//  Command Macros
// -----------------------------------------------------------------------------

#define CMD:%0(%1) \
    forward ss7_Cmd_%0(%1); \
    public  ss7_Cmd_%0(%1)

#define cmd:%0(%1) \
    forward ss7_Cmd_%0(%1, __[]); \
    public  ss7_Cmd_%0(%1, __[])

// -----------------------------------------------------------------------------
//  Alias Macros (Compile-time)
// -----------------------------------------------------------------------------

#define CMD_ALIAS(%0,%1) \
    CMD:%0(playerid, params[]) { return ss7_Cmd_%1(playerid, params); }

#if !defined ALIAS
    #define ALIAS(%0,%1) CMD_ALIAS(%0,%1)
#endif

// -----------------------------------------------------------------------------
//  Registration Macros
// -----------------------------------------------------------------------------

#define REG_CMD(%0)                 ss7_RegisterCmd(#%0)
#define REG_CMD_LVL(%0,%1)          ss7_RegisterCmd(#%0, %1)
#define REG_CMD_ADMIN(%0)           ss7_RegisterCmd(#%0, SS7_LEVEL_ADMIN)
#define REG_CMD_SADMIN(%0)          ss7_RegisterCmd(#%0, SS7_LEVEL_SADMIN)
#define REG_CMD_MOD(%0)             ss7_RegisterCmd(#%0, SS7_LEVEL_MOD)
#define REG_CMD_VIP(%0)             ss7_RegisterCmd(#%0, SS7_LEVEL_VIP)
#define REG_CMD_OWNER(%0)           ss7_RegisterCmd(#%0, SS7_LEVEL_OWNER)
#define REG_CMD_CD(%0,%1)           ss7_RegisterCmd(#%0); ss7_SetCmdCooldown(#%0, %1)

// -----------------------------------------------------------------------------
//  Group-Aware Registration Macros
// -----------------------------------------------------------------------------

#define SS7_REG_GROUP(%0,%1,%2)     ss7_RegisterCmdEx(#%0, %1, %2, 0)
#define SS7_REG_CD(%0,%1)           ss7_RegisterCmdEx(#%0, SS7_LEVEL_GUEST, -1, %1)
#define SS7_REG_CD_LVL(%0,%1,%2)    ss7_RegisterCmdEx(#%0, %1, -1, %2)
#define SS7_REG_FULL(%0,%1,%2,%3)   ss7_RegisterCmdEx(#%0, %1, %2, %3)

// -----------------------------------------------------------------------------
//  Metadata Macros
// -----------------------------------------------------------------------------

#define CMD_META(%0,%1,%2,%3)       ss7_SetMeta(ss7_GetCmdIndex(#%0), %1, %2, %3)
#define CMD_META2(%0,%1,%2)         ss7_SetMeta(ss7_GetCmdIndex(#%0), %1, %2, "")

// -----------------------------------------------------------------------------
//  Runtime Alias Macro
// -----------------------------------------------------------------------------

#define SS7_ALIAS(%0,%1)            ss7_AddAlias(ss7_GetCmdIndex(#%1), #%0)

// =============================================================================
//  END OF ss7_registry.inc
// =============================================================================