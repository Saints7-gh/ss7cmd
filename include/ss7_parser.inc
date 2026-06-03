// =============================================================================
//   ss7_parser.inc — Zero-Copy Cursor Parser
//   Module: Parser
//   Description: Extract command parameters without buffer copying.
//   Usage: p_s; at the start of any function that uses parsing.
// =============================================================================

#if defined _ss7_parser_included
    #endinput
#endif
#define _ss7_parser_included

#if !defined _ss7_core_included
    #error "[ss7cmd] ss7_core.inc must be included before ss7_parser.inc"
#endif

// =============================================================================
//  Internal Helpers
// =============================================================================

#define _ss7_SkipWS(%1,%2)  while ((%1)[%2] == ' ' || (%1)[%2] == '\t') (%2)++

// =============================================================================
//  Integer Parser
// =============================================================================

stock ss7_ParseInt(const params[], &cursor, &value)
{
    _ss7_SkipWS(params, cursor);
    if (params[cursor] == '\0') return 0;
    
    new neg = 0;
    if (params[cursor] == '-')
    {
        neg = 1;
        cursor++;
    }
    
    if (params[cursor] < '0' || params[cursor] > '9')
        return 0;
    
    value = 0;
    while (params[cursor] >= '0' && params[cursor] <= '9')
    {
        value = value * 10 + (params[cursor] - '0');
        cursor++;
    }
    
    if (neg) value = -value;
    return 1;
}

// =============================================================================
//  Float Parser
// =============================================================================

stock ss7_ParseFloat(const params[], &cursor, &Float:value)
{
    _ss7_SkipWS(params, cursor);
    if (params[cursor] == '\0') return 0;
    
    new neg = 0;
    if (params[cursor] == '-')
    {
        neg = 1;
        cursor++;
    }
    
    if (params[cursor] < '0' || params[cursor] > '9')
        return 0;
    
    new ipart = 0;
    while (params[cursor] >= '0' && params[cursor] <= '9')
    {
        ipart = ipart * 10 + (params[cursor] - '0');
        cursor++;
    }
    
    new fracInt = 0, fracDiv = 1;
    if (params[cursor] == '.')
    {
        cursor++;
        while (params[cursor] >= '0' && params[cursor] <= '9')
        {
            fracInt = fracInt * 10 + (params[cursor] - '0');
            fracDiv = fracDiv * 10;
            cursor++;
        }
    }
    
    value = float(ipart) + float(fracInt) / float(fracDiv);
    if (neg) value = -value;
    return 1;
}

// =============================================================================
//  Word Parser (Single Token)
// =============================================================================

stock ss7_ParseWord(const params[], &cursor, out[], maxlen = 32)
{
    _ss7_SkipWS(params, cursor);
    if (params[cursor] == '\0')
    {
        out[0] = '\0';
        return 0;
    }
    
    new j = 0;
    while (params[cursor] != '\0' && params[cursor] != ' ' && params[cursor] != '\t' && j < maxlen - 1)
        out[j++] = params[cursor++];
    out[j] = '\0';
    return j;
}

// =============================================================================
//  String Parser (Rest of Line)
// =============================================================================

stock ss7_ParseString(const params[], &cursor, out[], maxlen = 128)
{
    _ss7_SkipWS(params, cursor);
    if (params[cursor] == '\0')
    {
        out[0] = '\0';
        return 0;
    }
    
    new j = 0;
    while (params[cursor] != '\0' && j < maxlen - 1)
        out[j++] = params[cursor++];
    out[j] = '\0';
    return j;
}

// =============================================================================
//  Player Parser (ID or Partial Name)
// =============================================================================

stock ss7_ParsePlayer(const params[], &cursor)
{
    _ss7_SkipWS(params, cursor);
    if (params[cursor] == '\0')
        return INVALID_PLAYER_ID;
    
    if (params[cursor] >= '0' && params[cursor] <= '9')
    {
        new id = 0;
        while (params[cursor] >= '0' && params[cursor] <= '9')
        {
            id = id * 10 + (params[cursor] - '0');
            cursor++;
        }
        return IsPlayerConnected(id) ? id : INVALID_PLAYER_ID;
    }
    
    new token[MAX_PLAYER_NAME + 1];
    new j = 0;
    while (params[cursor] != '\0' && params[cursor] != ' ' && params[cursor] != '\t' && j < MAX_PLAYER_NAME)
        token[j++] = params[cursor++];
    token[j] = '\0';
    
    new found = INVALID_PLAYER_ID, matches = 0;
    new pname[MAX_PLAYER_NAME + 1];
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        GetPlayerName(i, pname, sizeof(pname));
        if (strfind(pname, token, true) != -1)
        {
            found = i;
            matches++;
            if (matches > 1) return INVALID_PLAYER_ID;
        }
    }
    return (matches == 1) ? found : INVALID_PLAYER_ID;
}

// =============================================================================
//  Parser Macros (Original - Full)
// =============================================================================

#define PARSE_BEGIN         new __c = 0

// _EX macros: parse into existing variables (no declaration)
#define PARSE_I_EX(%1,%2)   (ss7_ParseInt(%1, __c, %2))
#define PARSE_F_EX(%1,%2)   (ss7_ParseFloat(%1, __c, %2))
#define PARSE_W_EX(%1,%2)   (ss7_ParseWord(%1, __c, %2, sizeof(%2)) > 0)
#define PARSE_S_EX(%1,%2)   (ss7_ParseString(%1, __c, %2, sizeof(%2)) > 0)
#define PARSE_P_EX(%1,%2)   (((%2 = ss7_ParsePlayer(%1, __c)) != INVALID_PLAYER_ID))

#define PARSE_PI_EX(%1,%2,%3) \
    (((%2 = ss7_ParsePlayer(%1, __c)) != INVALID_PLAYER_ID) && ss7_ParseInt(%1, __c, %3))

#define PARSE_PF_EX(%1,%2,%3) \
    (((%2 = ss7_ParsePlayer(%1, __c)) != INVALID_PLAYER_ID) && ss7_ParseFloat(%1, __c, %3))

#define PARSE_PS_EX(%1,%2,%3) \
    (((%2 = ss7_ParsePlayer(%1, __c)) != INVALID_PLAYER_ID) && (ss7_ParseString(%1, __c, %3, sizeof(%3)) > 0))

#define PARSE_PIS_EX(%1,%2,%3,%4) \
    (((%2 = ss7_ParsePlayer(%1, __c)) != INVALID_PLAYER_ID) && ss7_ParseInt(%1, __c, %3) && (ss7_ParseString(%1, __c, %4, sizeof(%4)) > 0))

// Basic macros: declare + parse (use once per command)
#define PARSE_I(%1,%2)      new %2; PARSE_I_EX(%1,%2)
#define PARSE_F(%1,%2)      new Float:%2; PARSE_F_EX(%1,%2)
#define PARSE_W(%1,%2)      new %2[32]; PARSE_W_EX(%1,%2)
#define PARSE_S(%1,%2)      new %2[128]; PARSE_S_EX(%1,%2)
#define PARSE_P(%1,%2)      new %2; PARSE_P_EX(%1,%2)

// Compound macros: declare + parse multiple params
#define PARSE_PI(%1,%2,%3) \
    new %2, %3; \
    { \
        new __ss7_c = 0; \
        %2 = ss7_ParsePlayer(%1, __ss7_c); \
        ss7_ParseInt(%1, __ss7_c, %3); \
    }

#define PARSE_PF(%1,%2,%3) \
    new %2; new Float:%3; \
    { \
        new __ss7_c = 0; \
        %2 = ss7_ParsePlayer(%1, __ss7_c); \
        ss7_ParseFloat(%1, __ss7_c, %3); \
    }

#define PARSE_PW(%1,%2,%3) \
    new %2; new %3[32]; \
    { \
        new __ss7_c = 0; \
        %2 = ss7_ParsePlayer(%1, __ss7_c); \
        ss7_ParseWord(%1, __ss7_c, %3, sizeof(%3)); \
    }

#define PARSE_PS(%1,%2,%3) \
    new %2; new %3[128]; \
    { \
        new __ss7_c = 0; \
        %2 = ss7_ParsePlayer(%1, __ss7_c); \
        ss7_ParseString(%1, __ss7_c, %3, sizeof(%3)); \
    }

#define PARSE_PR(%1,%2,%3) PARSE_PS(%1,%2,%3)

#define PARSE_PIS(%1,%2,%3,%4) \
    new %2, %3; new %4[128]; \
    { \
        new __ss7_c = 0; \
        %2 = ss7_ParsePlayer(%1, __ss7_c); \
        ss7_ParseInt(%1, __ss7_c, %3); \
        ss7_ParseString(%1, __ss7_c, %4, sizeof(%4)); \
    }

// =============================================================================
//  Short Aliases (Clean, Modern Style)
// =============================================================================

// Start parsing
#define p_s         PARSE_BEGIN

// Basic aliases (declare + parse)
#define i(%0,%1)    PARSE_I(%0,%1)
#define f(%0,%1)    PARSE_F(%0,%1)
#define p(%0,%1)    PARSE_P(%0,%1)
#define s(%0,%1)    PARSE_S(%0,%1)
#define w(%0,%1)    PARSE_W(%0,%1)

// _EX aliases (parse into existing variables)
#define i_ex(%0,%1) PARSE_I_EX(%0,%1)
#define f_ex(%0,%1) PARSE_F_EX(%0,%1)
#define p_ex(%0,%1) PARSE_P_EX(%0,%1)
#define s_ex(%0,%1) PARSE_S_EX(%0,%1)
#define w_ex(%0,%1) PARSE_W_EX(%0,%1)

#define pi_ex(%0,%1,%2)  PARSE_PI_EX(%0,%1,%2)
#define pf_ex(%0,%1,%2)  PARSE_PF_EX(%0,%1,%2)
#define ps_ex(%0,%1,%2)  PARSE_PS_EX(%0,%1,%2)
#define pis_ex(%0,%1,%2,%3) PARSE_PIS_EX(%0,%1,%2,%3)

// Compound aliases (declare + parse)
#define pi(%0,%1,%2)    PARSE_PI(%0,%1,%2)
#define pf(%0,%1,%2)    PARSE_PF(%0,%1,%2)
#define ps(%0,%1,%2)    PARSE_PS(%0,%1,%2)
#define pis(%0,%1,%2,%3) PARSE_PIS(%0,%1,%2,%3)

// =============================================================================
//  End of ss7_parser.inc
// =============================================================================