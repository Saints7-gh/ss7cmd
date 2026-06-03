// =============================================================================
//   ss7cmd v1.0.0
//   File: benchmark_ss7cmd.pwn
//   Target: 100,000 Iterations Full Engine Stress-Test
// =============================================================================

#define SS7_ENABLE_ANTISPAM 0
#define SS7_ENABLE_COOLDOWN 0

#include <open.mp>
#include <ss7cmd>

#define ITERS 100000

// =============================================================================
//  DUMMY VARIABLES & COMMANDS
// =============================================================================

new g_IntOut;
new Float:g_FloatOut;
new g_StringOut[128];

CMD:bench_base(playerid, params[]) {
    return CMD_HANDLED;
}

CMD:bench_parse(playerid, params[]) {
    PARSE_BEGIN;
    PARSE_I_EX(params, g_IntOut);
    PARSE_F_EX(params, g_FloatOut);
    PARSE_S_EX(params, g_StringOut);
    return CMD_HANDLED;
}

CMD:bench_admin(playerid, params[]) {
    return CMD_HANDLED;
}

forward BenchReflection(idx, const name[], const desc[], level, group);
public BenchReflection(idx, const name[], const desc[], level, group) 
{
    return 1;
}

main() 
{
   
}

// =============================================================================
//  BENCHMARK EXECUTION (OnGameModeInit)
// =============================================================================

public OnGameModeInit()
{
    // 1. Inisialisasi TITAN Engine
    ss7_RuntimeInit();

    // 2. Registrasi Command
    REG_CMD(bench_base);
    REG_CMD(bench_parse);
    REG_CMD_ADMIN(bench_admin);
    SS7_ALIAS(bench_alias, bench_base);

    // 3. Set Status Player Dummy (ID 0)
    ss7_SetPlayerLevel(0, SS7_LEVEL_GUEST);
    ss7_SetPlayerGroup(0, 1);

    print("\n");
    print("===============================================================");
    printf(" [TITAN v1] BENCHMARK PROTOCOL — %d ITERATIONS", ITERS);
    print("===============================================================");

    new t, ms;
    new Float:us;

    // --- TEST 1: Hash Generation (FNV-1a Engine) ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_HashString("bench_base");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] String Hash (FNV-1a)       : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 2: O(1) Table Lookup ---
    new h = ss7_HashString("bench_base");
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_FindCmdByHashPublic(h, "bench_base");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Cmd Lookup O(1) Array      : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 3: Full Dispatch (Command Without Parameters) ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_Dispatch(0, "/bench_base");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Full Dispatch (Empty Cmd)  : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 4: Full Dispatch (Alias O(1) Traversal) ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_Dispatch(0, "/bench_alias");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Full Dispatch (Alias O(1)) : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 5: Full Dispatch (+ Parser Extraction) ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_Dispatch(0, "/bench_parse 1337 3.1415 TITAN_IS_FAST");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Full Dispatch (+3 Params)  : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 6: Full Dispatch (Unknown Command / NotFound) ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_Dispatch(0, "/does_not_exist");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Dispatch (Not Found Skip)  : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 7: Full Dispatch (Permission Denied Block) ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_Dispatch(0, "/bench_admin");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Dispatch (Perm Blocked)    : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 8: Parser Engine Khusus (Zero-Copy Macros) ---
    new pstr[] = "42 99.9 ZERO_COPY_ENGINE";
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        PARSE_BEGIN;
        new a; new Float:b; new c[32];
        PARSE_I_EX(pstr, a);
        PARSE_F_EX(pstr, b);
        PARSE_S_EX(pstr, c);
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Parse Macros (Zero-Copy)   : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 9: Bitwise Permission Evaluation ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_PlayerHasPerm(0, 12); 
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Bitwise Perm Eval (O(1))   : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 10: Typo Suggestion (Levenshtein Distance Engine) ---
    new sugg[SS7_MAX_CMD_NAME];
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        _ss7_Internal_ResetPlayer(0); 
        ss7_GetSuggestion(0, "bnch_bse", sugg, sizeof(sugg));
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Typo Suggest (Levenshtein) : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 11: Reflection API ---
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_ForEachCmd("BenchReflection");
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] Reflection (ForEachCmd)    : %d ms \t(%.4f µs/iter)", ms, us);

    // --- TEST 12: UI Formatting Engine ---
    new ui_out[32];
    t = GetTickCount();
    for(new i = 0; i < ITERS; i++) {
        ss7_FormatNumber(123456789, ui_out, sizeof(ui_out));  
        ss7_FormatMoney(987654321, ui_out, sizeof(ui_out));   
        ss7_FormatTime(3665, ui_out);                         
    }
    ms = GetTickCount() - t;
    us = (float(ms) * 1000.0) / float(ITERS);
    printf(" [✔] UI Formatting (3 Ops/iter) : %d ms \t(%.4f µs/iter)", ms, us);

    print("===============================================================");
    
    ss7_PrintMetrics();

    return 1;
}