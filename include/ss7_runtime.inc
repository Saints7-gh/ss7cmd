// =============================================================================
//   ss7_runtime.inc — Runtime Diagnostics & Profiler
//   Performance benchmarking and profiling for the TITAN framework.
// =============================================================================

#if defined _ss7_runtime_included
    #endinput
#endif
#define _ss7_runtime_included

#if !defined _ss7_core_included
    #error "[ss7rt] ss7_core.inc must be included before ss7_runtime.inc"
#endif

#define SS7RT_MAX_PROBES        16
#define SS7RT_PROBE_LABEL_LEN   32
#define SS7RT_HISTORY_SIZE      64
#define SS7RT_MAX_PROFILE_SLOTS 8

#define SS7RT_SLOT_HASH             (0)
#define SS7RT_SLOT_LOOKUP           (1)
#define SS7RT_SLOT_PARSE_INPUT      (2)
#define SS7RT_SLOT_MIDDLEWARE       (3)
#define SS7RT_SLOT_EXECUTE          (4)
#define SS7RT_SLOT_DISPATCH_FULL    (5)
#define SS7RT_SLOT_PERM_CHECK       (6)
#define SS7RT_SLOT_COOLDOWN         (7)

static _ss7rt_ProbeLabel  [SS7RT_MAX_PROBES][SS7RT_PROBE_LABEL_LEN];
static _ss7rt_ProbeStart  [SS7RT_MAX_PROBES];
static _ss7rt_ProbeTotal  [SS7RT_MAX_PROBES];
static _ss7rt_ProbeCount  [SS7RT_MAX_PROBES];
static _ss7rt_ProbeMin    [SS7RT_MAX_PROBES];
static _ss7rt_ProbeMax    [SS7RT_MAX_PROBES];
static _ss7rt_ProbeActive [SS7RT_MAX_PROBES];
static _ss7rt_ProbeUsed   = 0;

static _ss7rt_History     [SS7RT_HISTORY_SIZE];
static _ss7rt_HistHead    = 0;
static _ss7rt_HistCount   = 0;

static _ss7rt_SlotStart   [SS7RT_MAX_PROFILE_SLOTS];
static _ss7rt_SlotTotal   [SS7RT_MAX_PROFILE_SLOTS];
static _ss7rt_SlotCalls   [SS7RT_MAX_PROFILE_SLOTS];
static _ss7rt_SlotMin     [SS7RT_MAX_PROFILE_SLOTS];
static _ss7rt_SlotMax     [SS7RT_MAX_PROFILE_SLOTS];

static _ss7rt_TotalDispatches = 0;
static _ss7rt_TotalFound      = 0;
static _ss7rt_TotalNotFound   = 0;
static _ss7rt_TotalBlocked    = 0;
static _ss7rt_StartTick       = 0;

// =============================================================================
//  INITIALIZATION
// =============================================================================

stock ss7_RuntimeInit()
{
    _ss7rt_StartTick = GetTickCount();
    _ss7rt_ProbeUsed = 0;
    _ss7rt_HistHead = 0;
    _ss7rt_HistCount = 0;
    
    for (new i = 0; i < SS7RT_MAX_PROBES; i++)
    {
        _ss7rt_ProbeLabel[i][0] = '\0';
        _ss7rt_ProbeStart[i] = 0;
        _ss7rt_ProbeTotal[i] = 0;
        _ss7rt_ProbeCount[i] = 0;
        _ss7rt_ProbeMin[i] = 0x7FFFFFFF;
        _ss7rt_ProbeMax[i] = 0;
        _ss7rt_ProbeActive[i] = 0;
    }
    
    for (new i = 0; i < SS7RT_MAX_PROFILE_SLOTS; i++)
    {
        _ss7rt_SlotStart[i] = 0;
        _ss7rt_SlotTotal[i] = 0;
        _ss7rt_SlotCalls[i] = 0;
        _ss7rt_SlotMin[i] = 0x7FFFFFFF;
        _ss7rt_SlotMax[i] = 0;
    }
    
    _ss7rt_TotalDispatches = 0;
    _ss7rt_TotalFound = 0;
    _ss7rt_TotalNotFound = 0;
    _ss7rt_TotalBlocked = 0;
    
    printf("[ss7rt] Runtime diagnostics ready.");
    return 1;
}

// =============================================================================
//  BENCHMARK API
// =============================================================================

stock ss7_BenchmarkStart(const label[])
{
    for (new i = 0; i < _ss7rt_ProbeUsed; i++)
    {
        if (!strcmp(_ss7rt_ProbeLabel[i], label, true))
        {
            _ss7rt_ProbeStart[i] = GetTickCount();
            _ss7rt_ProbeActive[i] = 1;
            return i;
        }
    }
    
    if (_ss7rt_ProbeUsed >= SS7RT_MAX_PROBES) return -1;
    new idx = _ss7rt_ProbeUsed++;
    strcat(_ss7rt_ProbeLabel[idx], label, SS7RT_PROBE_LABEL_LEN);
    _ss7rt_ProbeStart[idx] = GetTickCount();
    _ss7rt_ProbeMin[idx] = 0x7FFFFFFF;
    _ss7rt_ProbeMax[idx] = 0;
    _ss7rt_ProbeActive[idx] = 1;
    return idx;
}

stock ss7_BenchmarkEnd(const label[], iters = 1)
{
    new now = GetTickCount();
    for (new i = 0; i < _ss7rt_ProbeUsed; i++)
    {
        if (!strcmp(_ss7rt_ProbeLabel[i], label, true) && _ss7rt_ProbeActive[i])
        {
            new delta = now - _ss7rt_ProbeStart[i];
            new us_per_iter = (delta * 1000) / iters;
            
            _ss7rt_ProbeTotal[i] += delta;
            _ss7rt_ProbeCount[i]++;
            _ss7rt_ProbeActive[i] = 0;
            if (delta < _ss7rt_ProbeMin[i]) _ss7rt_ProbeMin[i] = delta;
            if (delta > _ss7rt_ProbeMax[i]) _ss7rt_ProbeMax[i] = delta;
            
            _ss7rt_History[_ss7rt_HistHead] = delta;
            _ss7rt_HistHead = (_ss7rt_HistHead + 1) & (SS7RT_HISTORY_SIZE - 1);
            if (_ss7rt_HistCount < SS7RT_HISTORY_SIZE) _ss7rt_HistCount++;
            
            return us_per_iter;
        }
    }
    return -1;
}

stock ss7_BenchmarkEndIdx(idx, iters = 1)
{
    if (idx < 0 || idx >= _ss7rt_ProbeUsed) return -1;
    if (!_ss7rt_ProbeActive[idx]) return -1;
    
    new now = GetTickCount();
    new delta = now - _ss7rt_ProbeStart[idx];
    new us_per_iter = (delta * 1000) / iters;
    
    _ss7rt_ProbeTotal[idx] += delta;
    _ss7rt_ProbeCount[idx]++;
    _ss7rt_ProbeActive[idx] = 0;
    if (delta < _ss7rt_ProbeMin[idx]) _ss7rt_ProbeMin[idx] = delta;
    if (delta > _ss7rt_ProbeMax[idx]) _ss7rt_ProbeMax[idx] = delta;
    
    _ss7rt_History[_ss7rt_HistHead] = delta;
    _ss7rt_HistHead = (_ss7rt_HistHead + 1) & (SS7RT_HISTORY_SIZE - 1);
    if (_ss7rt_HistCount < SS7RT_HISTORY_SIZE) _ss7rt_HistCount++;
    
    return us_per_iter;
}

// =============================================================================
//  PROFILE SLOTS API
// =============================================================================

stock ss7_ProfileBegin(slot)
{
    if (slot < 0 || slot >= SS7RT_MAX_PROFILE_SLOTS) return;
    _ss7rt_SlotStart[slot] = GetTickCount();
}

stock ss7_ProfileEnd(slot)
{
    if (slot < 0 || slot >= SS7RT_MAX_PROFILE_SLOTS) return;
    new delta = GetTickCount() - _ss7rt_SlotStart[slot];
    _ss7rt_SlotTotal[slot] += delta;
    _ss7rt_SlotCalls[slot]++;
    if (delta < _ss7rt_SlotMin[slot]) _ss7rt_SlotMin[slot] = delta;
    if (delta > _ss7rt_SlotMax[slot]) _ss7rt_SlotMax[slot] = delta;
}

// =============================================================================
//  EVENT COUNTERS
// =============================================================================

stock ss7_RT_CountDispatch() { _ss7rt_TotalDispatches++; }
stock ss7_RT_CountFound()    { _ss7rt_TotalFound++; }
stock ss7_RT_CountNotFound() { _ss7rt_TotalNotFound++; }
stock ss7_RT_CountBlocked()  { _ss7rt_TotalBlocked++; }

// =============================================================================
//  METRICS PRINTER
// =============================================================================

stock ss7_PrintMetrics()
{
    new uptime = GetTickCount() - _ss7rt_StartTick;
    printf("[ss7rt] ═══════════════════════════════════════════════════════════");
    printf("[ss7rt] TITAN RUNTIME METRICS — Uptime: %d ms", uptime);
    printf("[ss7rt] Dispatches: %d | Found: %d | NotFound: %d | Blocked: %d",
        _ss7rt_TotalDispatches, _ss7rt_TotalFound, _ss7rt_TotalNotFound, _ss7rt_TotalBlocked);
    
    printf("[ss7rt] ─── Named Probes ───");
    for (new i = 0; i < _ss7rt_ProbeUsed; i++)
    {
        if (_ss7rt_ProbeCount[i] == 0) continue;
        new avg = _ss7rt_ProbeTotal[i] / _ss7rt_ProbeCount[i];
        new low = (_ss7rt_ProbeMin[i] == 0x7FFFFFFF) ? 0 : _ss7rt_ProbeMin[i];
        new high = _ss7rt_ProbeMax[i];
        printf("[ss7rt] %-14s hits=%-6d avg=%-4dms low=%-4dms high=%dms",
            _ss7rt_ProbeLabel[i], _ss7rt_ProbeCount[i], avg, low, high);
    }
    printf("[ss7rt] ═══════════════════════════════════════════════════════════");
}

// =============================================================================
//  END OF ss7_runtime.inc
// =============================================================================