// =============================================================================
//   ss7_ui.inc
//   Module: UI
//   Description: Messaging, formatting, progress bar, dialogs, player helpers.
// =============================================================================

#if defined _ss7_ui_included
    #endinput
#endif
#define _ss7_ui_included

#if !defined _ss7_core_included
    #error "[ss7ui] ss7_core.inc must be included before ss7_ui.inc"
#endif

// -----------------------------------------------------------------------------
//  Color Palette
// -----------------------------------------------------------------------------
#if !defined SS7_COLOR_ERROR
    #define SS7_COLOR_ERROR      0xFF3333FF
#endif
#if !defined SS7_COLOR_SUCCESS
    #define SS7_COLOR_SUCCESS    0x44FF88FF
#endif
#if !defined SS7_COLOR_INFO
    #define SS7_COLOR_INFO       0x66CCFFFF
#endif
#if !defined SS7_COLOR_WARN
    #define SS7_COLOR_WARN       0xFFCC00FF
#endif
#if !defined SS7_COLOR_SYNTAX
    #define SS7_COLOR_SYNTAX     0xFF9900FF
#endif
#if !defined SS7_COLOR_HEADER
    #define SS7_COLOR_HEADER     0x00BFFFFF
#endif
#if !defined SS7_COLOR_DIVIDER
    #define SS7_COLOR_DIVIDER    0x444444FF
#endif
#if !defined SS7_COLOR_VALUE
    #define SS7_COLOR_VALUE      0xFFFF00FF
#endif
#if !defined SS7_COLOR_NUMBER
    #define SS7_COLOR_NUMBER     0x88FF88FF
#endif

// -----------------------------------------------------------------------------
//  Messaging
// -----------------------------------------------------------------------------
stock ss7_SendError(playerid, const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "!  %s", msg);
    SendClientMessage(playerid, SS7_COLOR_ERROR, buf);
}

stock ss7_SendSuccess(playerid, const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "+  %s", msg);
    SendClientMessage(playerid, SS7_COLOR_SUCCESS, buf);
}

stock ss7_SendInfo(playerid, const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "i  %s", msg);
    SendClientMessage(playerid, SS7_COLOR_INFO, buf);
}

stock ss7_SendWarn(playerid, const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "?  %s", msg);
    SendClientMessage(playerid, SS7_COLOR_WARN, buf);
}

stock ss7_SendSyntax(playerid, const usage[])
{
    new buf[160];
    format(buf, sizeof(buf), ">  Usage: %s", usage);
    SendClientMessage(playerid, SS7_COLOR_SYNTAX, buf);
    return 0;
}

stock ss7_SendHeader(playerid, const title[], color = SS7_COLOR_HEADER)
{
    new line[72];
    format(line, sizeof(line), "— %s —", title);
    SendClientMessage(playerid, color, line);
}

stock ss7_SendDivider(playerid, color = SS7_COLOR_DIVIDER)
{
    SendClientMessage(playerid, color, "————————————————————————————————");
}

stock ss7_SendBlank(playerid)
{
    SendClientMessage(playerid, 0x000000FF, " ");
}

// -----------------------------------------------------------------------------
//  Broadcast
// -----------------------------------------------------------------------------
stock ss7_BroadcastError(const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "!  %s", msg);
    SendClientMessageToAll(SS7_COLOR_ERROR, buf);
}

stock ss7_BroadcastSuccess(const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "+  %s", msg);
    SendClientMessageToAll(SS7_COLOR_SUCCESS, buf);
}

stock ss7_BroadcastInfo(const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "i  %s", msg);
    SendClientMessageToAll(SS7_COLOR_INFO, buf);
}

stock ss7_BroadcastWarn(const msg[])
{
    new buf[160];
    format(buf, sizeof(buf), "?  %s", msg);
    SendClientMessageToAll(SS7_COLOR_WARN, buf);
}

// -----------------------------------------------------------------------------
//  Formatting
// -----------------------------------------------------------------------------
stock ss7_FormatNumber(value, out[], maxlen = 32)
{
    new tmp[24], result[32];
    new neg = (value < 0);
    if (neg) value = -value;

    format(tmp, sizeof(tmp), "%d", value);
    new len = strlen(tmp);
    new rlen = 0;

    for (new i = len - 1, cnt = 0; i >= 0; i--, cnt++)
    {
        if (cnt > 0 && (cnt % 3) == 0)
            result[rlen++] = ',';
        result[rlen++] = tmp[i];
    }
    result[rlen] = '\0';

    new i = 0, j = rlen - 1;
    while (i < j)
    {
        new t = result[i];
        result[i] = result[j];
        result[j] = t;
        i++; j--;
    }

    out[0] = '\0';
    if (neg) strcat(out, "-", maxlen);
    strcat(out, result, maxlen);
}

stock ss7_FormatMoney(value, out[], maxlen = 32)
{
    new numstr[32];
    ss7_FormatNumber(value, numstr, sizeof(numstr));
    format(out, maxlen, "$%s", numstr);
}

stock ss7_FormatFloat(Float:value, out[], decimals = 2)
{
    new fmt[12];
    format(fmt, sizeof(fmt), "%%.%df", decimals);
    format(out, 32, fmt, value);
}

stock ss7_FormatPercent(Float:ratio, out[])
{
    format(out, 16, "%.1f%%", ratio * 100.0);
}

stock ss7_FormatTime(seconds, out[])
{
    new h = seconds / 3600;
    new m = (seconds % 3600) / 60;
    new s = seconds % 60;

    if (h > 0)
        format(out, 32, "%dh %dm %ds", h, m, s);
    else if (m > 0)
        format(out, 32, "%dm %ds", m, s);
    else
        format(out, 32, "%ds", s);
}

stock ss7_FormatMs(ms, out[])
{
    ss7_FormatTime(ms / 1000, out);
}

// -----------------------------------------------------------------------------
//  Progress Bar
// -----------------------------------------------------------------------------
stock ss7_ProgressBar(Float:percent, out[], width = 20)
{
    new filled = floatround(percent * width / 100.0);
    new empty = width - filled;

    out[0] = '[';
    for (new i = 0; i < filled; i++) out[1 + i] = '|';
    for (new i = 0; i < empty; i++) out[1 + filled + i] = ' ';
    out[1 + width] = ']';
    out[2 + width] = '\0';
}

// -----------------------------------------------------------------------------
//  Player Helpers
// -----------------------------------------------------------------------------
stock ss7_GetPlayerNameEx(playerid, out[], len = MAX_PLAYER_NAME)
{
    GetPlayerName(playerid, out, len);
}

stock ss7_GetColoredName(playerid, out[], len)
{
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof(name));

    new level = ss7_GetPlayerLevel(playerid);
    new hex;

    if (level >= SS7_LEVEL_OWNER)       hex = 0xFF2222;
    else if (level >= SS7_LEVEL_SADMIN) hex = 0xFF8800;
    else if (level >= SS7_LEVEL_ADMIN)  hex = 0xFF4444;
    else if (level >= SS7_LEVEL_MOD)    hex = 0x44BBFF;
    else if (level >= SS7_LEVEL_VIP)    hex = 0xFFD700;
    else                                hex = 0xCCCCCC;

    new tag[10];
    format(tag, sizeof(tag), "{%06X}", hex);
    format(out, len, "%s%s", tag, name);
}

stock ss7_GetLevelName(level, out[], len)
{
    out[0] = '\0';
    if      (level >= SS7_LEVEL_OWNER)  strcat(out, "Owner",      len);
    else if (level >= SS7_LEVEL_SADMIN) strcat(out, "SuperAdmin", len);
    else if (level >= SS7_LEVEL_ADMIN)  strcat(out, "Admin",      len);
    else if (level >= SS7_LEVEL_MOD)    strcat(out, "Moderator",  len);
    else if (level >= SS7_LEVEL_HELPER) strcat(out, "Helper",     len);
    else if (level >= SS7_LEVEL_VIP)    strcat(out, "VIP",        len);
    else                                strcat(out, "Player",     len);
}

// -----------------------------------------------------------------------------
//  Dialog Builders
// -----------------------------------------------------------------------------
stock ss7_ShowMsgBox(playerid, const title[], const message[], const button[] = "OK")
{
    ShowPlayerDialog(playerid, 0, DIALOG_STYLE_MSGBOX, title, message, button, "");
}

stock ss7_ShowInput(playerid, const title[], const message[], const button[] = "Submit")
{
    return ShowPlayerDialog(playerid, 1, DIALOG_STYLE_INPUT, title, message, button, "Cancel");
}

stock ss7_ShowList(playerid, const title[], const items[], const button[] = "Select")
{
    return ShowPlayerDialog(playerid, 2, DIALOG_STYLE_LIST, title, items, button, "Cancel");
}

stock ss7_ShowTablist(playerid, const title[], const items[], const button[] = "Select")
{
    return ShowPlayerDialog(playerid, 3, DIALOG_STYLE_TABLIST, title, items, button, "Cancel");
}

stock ss7_ShowTablistHeaders(playerid, const title[], const items[], const button[] = "Select")
{
    return ShowPlayerDialog(playerid, 4, DIALOG_STYLE_TABLIST_HEADERS, title, items, button, "Cancel");
}

stock ss7_ShowPassword(playerid, const title[], const message[], const button[] = "Submit")
{
    return ShowPlayerDialog(playerid, 5, DIALOG_STYLE_PASSWORD, title, message, button, "Cancel");
}

// -----------------------------------------------------------------------------
//  Table Display
// -----------------------------------------------------------------------------
stock ss7_TableHeader(playerid, const header[])
{
    ss7_SendHeader(playerid, header);
}

stock ss7_TableRow(playerid, const row[])
{
    SendClientMessage(playerid, SS7_COLOR_INFO, row);
}

// -----------------------------------------------------------------------------
//  Message Shortcuts
// -----------------------------------------------------------------------------
#define ss7_Error(%1,%2)    ss7_SendError(%1, %2)
#define ss7_Success(%1,%2)  ss7_SendSuccess(%1, %2)
#define ss7_Info(%1,%2)     ss7_SendInfo(%1, %2)
#define ss7_Warn(%1,%2)     ss7_SendWarn(%1, %2)
#define ss7_Syntax(%1,%2)   ss7_SendSyntax(%1, %2)

// =============================================================================
//  END OF ss7_ui.inc
// =============================================================================