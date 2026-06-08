// =============================================================================
//   ss7_hook.inc — SA-MP / open.mp Standard
//
//   Zero boilerplate integration:
//     - Automatically initializes ss7cmd   
// =============================================================================

#if defined _ss7_hook_included
    #endinput
#endif
#define _ss7_hook_included

#if !defined _ss7_core_included
    #error "[ss7hook] ss7_core.inc must be included before ss7_hook.inc"
#endif

// =============================================================================
//  OnGameModeInit
// =============================================================================
public OnGameModeInit()
{
    ss7_Init();
    ss7cmdlist();

    #if defined ss7_gm_OnGameModeInit
        ss7_gm_OnGameModeInit();
    #endif
    return 1;
}

#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif

#define OnGameModeInit ss7_hook_OnGameModeInit
forward ss7_hook_OnGameModeInit();

#undef OnGameModeInit
#define OnGameModeInit ss7_gm_OnGameModeInit

#if defined ss7_gm_OnGameModeInit
    forward ss7_gm_OnGameModeInit();
#endif

// =============================================================================
//  OnGameModeExit
// =============================================================================
public OnGameModeExit()
{
    #if defined ss7_gm_OnGameModeExit
        return ss7_gm_OnGameModeExit();
    #else
        return 1;
    #endif
}

#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif

#define OnGameModeExit ss7_hook_OnGameModeExit
forward ss7_hook_OnGameModeExit();

#undef OnGameModeExit
#define OnGameModeExit ss7_gm_OnGameModeExit

#if defined ss7_gm_OnGameModeExit
    forward ss7_gm_OnGameModeExit();
#endif

// =============================================================================
//  OnPlayerCommandText
// =============================================================================
public OnPlayerCommandText(playerid, cmdtext[])
{
    if (ss7_Dispatch(playerid, cmdtext))
        return 1;

    #if SS7_TYPO_SUGGEST
        new cmdname[SS7_MAX_CMD_NAME];
        new pos = 1, idx = 0;
        while (cmdtext[pos] != ' ' && cmdtext[pos] != '\0' && idx < SS7_MAX_CMD_NAME - 1)
            cmdname[idx++] = cmdtext[pos++];
        cmdname[idx] = '\0';

        new suggestion[SS7_MAX_CMD_NAME];
        if (ss7_GetSuggestion(playerid, cmdname, suggestion, sizeof(suggestion)) != SS7_INVALID_CMD)
        {
            new msg[128];
            format(msg, sizeof(msg), "Unknown command. Did you mean /%s?", suggestion);
            SendClientMessage(playerid, 0xFFCC00FF, msg);
            return 1;
        }
    #endif

    #if defined ss7_gm_OnPlayerCommandText
        return ss7_gm_OnPlayerCommandText(playerid, cmdtext);
    #else
        return 0;
    #endif
}

#if defined _ALS_OnPlayerCommandText
    #undef OnPlayerCommandText
#else
    #define _ALS_OnPlayerCommandText
#endif

#define OnPlayerCommandText ss7_hook_OnPlayerCommandText
forward ss7_hook_OnPlayerCommandText(playerid, cmdtext[]);

#undef OnPlayerCommandText
#define OnPlayerCommandText ss7_gm_OnPlayerCommandText

#if defined ss7_gm_OnPlayerCommandText
    forward ss7_gm_OnPlayerCommandText(playerid, cmdtext[]);
#endif

// =============================================================================
//  OnPlayerConnect
// =============================================================================
public OnPlayerConnect(playerid)
{
    _ss7_Internal_ResetPlayer(playerid);

    #if defined ss7_gm_OnPlayerConnect
        return ss7_gm_OnPlayerConnect(playerid);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif

#define OnPlayerConnect ss7_hook_OnPlayerConnect
forward ss7_hook_OnPlayerConnect(playerid);

#undef OnPlayerConnect
#define OnPlayerConnect ss7_gm_OnPlayerConnect

#if defined ss7_gm_OnPlayerConnect
    forward ss7_gm_OnPlayerConnect(playerid);
#endif

// =============================================================================
//  OnPlayerDisconnect
// =============================================================================
public OnPlayerDisconnect(playerid, reason)
{
    _ss7_Internal_ResetPlayer(playerid);

    #if defined ss7_gm_OnPlayerDisconnect
        return ss7_gm_OnPlayerDisconnect(playerid, reason);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif

#define OnPlayerDisconnect ss7_hook_OnPlayerDisconnect
forward ss7_hook_OnPlayerDisconnect(playerid, reason);

#undef OnPlayerDisconnect
#define OnPlayerDisconnect ss7_gm_OnPlayerDisconnect

#if defined ss7_gm_OnPlayerDisconnect
    forward ss7_gm_OnPlayerDisconnect(playerid, reason);
#endif

// =============================================================================
//  OnDialogResponse
// =============================================================================
public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    #if defined ss7_gm_OnDialogResponse
        return ss7_gm_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
    #else
        return 0;
    #endif
}

#if defined _ALS_OnDialogResponse
    #undef OnDialogResponse
#else
    #define _ALS_OnDialogResponse
#endif

#define OnDialogResponse ss7_hook_OnDialogResponse
forward ss7_hook_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);

#undef OnDialogResponse
#define OnDialogResponse ss7_gm_OnDialogResponse

#if defined ss7_gm_OnDialogResponse
    forward ss7_gm_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
#endif

// =============================================================================
//  END OF ss7_hook.inc
// =============================================================================