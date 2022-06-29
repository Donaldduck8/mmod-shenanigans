#ifdef _WIN32
#include "Windows.h"
#include "Psapi.h"
#pragma comment(lib, "psapi.lib")
#elif defined (__linux__)
#elif defined (__APPLE__)
#endif

#include "tickset.h"
#include "tier0/platform.h"

float* TickSet::interval_per_tick = nullptr;
bool TickSet::m_bInGameUpdate = false;
const Tickrate TickSet::s_DefinedRates[] = {
    { 0.015f, "66" },
    { 0.01f, "100" }
};
Tickrate TickSet::m_trCurrent = s_DefinedRates[TICKRATE_66];

#ifdef __linux__
int GetModuleInformation_Linux(const char *name, void **base, size_t *length)
{
    // this is the only way to do this on linux, lol
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return 1;

    char buf[PATH_MAX+100];
    while (!feof(f))
    {
        if (!fgets(buf, sizeof(buf), f))
            break;

	char *tmp = strrchr(buf, '\n');
	if (tmp)
            *tmp = '\0';

	char *mapname = strchr(buf, '/');
	if (!mapname)
	    continue;

        char perm[5];
        unsigned long begin, end;
        sscanf(buf, "%lx-%lx %4s", &begin, &end, perm);

        if (strcmp(basename(mapname), name) == 0 && perm[0] == 'r' && perm[2] == 'x')
        {
            *base = (void*)begin;
            *length = (size_t)end-begin;
            fclose(f);
            return 0;
        }
    }

    fclose(f);
    return 2;
}
#endif // __linux__

inline bool TickSet::DataCompare(const unsigned char* data, const unsigned char* pattern, const char* mask)
{
    for (; *mask != 0; ++data, ++pattern, ++mask)
        if (*mask == 'x' && *data != *pattern)
            return false;

    return (*mask == 0);
}

void* TickSet::FindPattern(const void* start, size_t length, const unsigned char* pattern, const char* mask)
{
    auto maskLength = strlen(mask);
    for (size_t i = 0; i <= length - maskLength; ++i)
    {
        auto addr = reinterpret_cast<const unsigned char*>(start)+i;
        if (DataCompare(addr, pattern, mask))
            return const_cast<void*>(reinterpret_cast<const void*>(addr));
    }

    return nullptr;
}

bool TickSet::TickInit()
{
#ifdef _WIN32
    HMODULE handle = GetModuleHandleA("engine.dll");
    if (!handle)
        return false;    
    
    MODULEINFO info;
    GetModuleInformation(GetCurrentProcess(), handle, &info, sizeof(info));

    auto moduleBase = info.lpBaseOfDll;
    auto moduleSize = info.SizeOfImage;

    unsigned char pattern[] = { 0x8B, 0x0D, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0xFF, '?', 0xD9, 0x15, '?', '?', '?', '?', 0xDD, 0x05, '?', '?', '?', '?', 0xDB, 0xF1, 0xDD, 0x05, '?', '?', '?', '?', 0x77, 0x08, 0xD9, 0xCA, 0xDB, 0xF2, 0x76, 0x1F, 0xD9, 0xCA };
    auto p = reinterpret_cast<uintptr_t>(FindPattern(moduleBase, moduleSize, pattern, "xx????????????x?xx????xx????xxxx????xxxxxxxxxx"));
    if (p)
        interval_per_tick = *reinterpret_cast<float**>(p + 18);

    // TRIKZ TODO: MOVE THIS TO SEPARATE FILE- SHOULD NOT BE IN TICKSET
    // Prevent the culling of skyboxes at high FOVs
    // https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_warp.cpp#L315
    // TODO: Use a value derived from FOV instead
    unsigned char skyboxpattern[] = {0xF3, 0x0F, 0x59, 0x15, '?',  '?',  '?',  '?',
                               0xF3, 0x0F, 0x58, 0xC1, 0xF3, 0x0F, 0x10, 0x0D};
    auto addr = reinterpret_cast<uintptr_t>(FindPattern(moduleBase, moduleSize, skyboxpattern, "xxxx????xxxxxxxx"));

    if (addr)
    {
        // The value is stored in the data segment so it needs write permission
        float *fValue = *reinterpret_cast<float **>(addr + 16);

        // 0x40 is read,write,execute
        unsigned long iOldProtection, iNewProtection = 0x40;

        if (VirtualProtect((void *)fValue, sizeof(float), iNewProtection, &iOldProtection))
        {
            *fValue = -1;

            // Restore old protections
            VirtualProtect((void *)fValue, sizeof(float), iOldProtection, &iNewProtection);
        }
    }

    // Prevent the FCVAR_NOT_CONNECTED flag from being checked when on a server
    // This fixes commands such as fps_max and cl_interp not working when playing
    // on a map to be closer in-line with how it works in CS:GO.
    unsigned char notconnectedpattern[] = {0x8B, 0x06, 0x8B, 0xCE, 0x68, '?',  '?',  '?',  '?',  0x8B, 0x40, 0x08, 0xFF, 0xD0,
                                 0x84, 0xC0, 0x74, '?',  0x83, 0x3D, '?',  '?',  '?',  '?',  '?',  0x7C, '?',  0x8B,
                                 0x06, 0x8B, 0xCE, 0x68, '?',  '?',  '?',  '?',  0x8B, 0x40, 0x08, 0xFF, 0xD0, 0x84,
                                 0xC0, 0x74, '?',  0x8B, 0x0D, '?',  '?',  '?',  '?',  0x85, 0xC9, 0x74, '?',  0x8B,
                                 0x11, 0x8D, 0x46, 0x18, 0x50, 0x8B, 0x82, 0x30, 0x01, 0x00, 0x00, 0xFF, 0xD0, 0x84,
                                 0xC0, 0x75, '?',  0x8B, 0x06, 0x8B, 0xCE, 0xFF, 0x50, 0x10, 0x50, 0x68, '?',  '?',
                                 '?',  '?',  0xFF, 0x15, '?',  '?',  '?',  '?',  0x83, 0xC4, '?',  0xB0, '?'};
                                 
    void* notconnectedAddr = FindPattern(moduleBase, moduleSize, notconnectedpattern,
             "xxxxx????xxxxxxxx?xx?????x?xxxxx????xxxxxxxx?xx????xxx?xxxxxxxxxxxxxxxxx?xxxxxxxxx????xx????xx?x?");

    if (notconnectedAddr)
    {
        notconnectedAddr = (uintptr_t *)notconnectedAddr + 5;

        unsigned long iOldProtection, iNewProtection = 0x40;
        Msg("Hey we found it!\n");
        if (VirtualProtect(notconnectedAddr, 1, iNewProtection, &iOldProtection))
        {
            Q_memcpy(notconnectedAddr, "", 1);

            VirtualProtect(notconnectedAddr, 1, iOldProtection, &iNewProtection);
        }
    }

#elif defined (__linux__)
    void *base;
    size_t length;
    if (GetModuleInformation_Linux("engine.so", &base, &length))
	return false;
    
    // mov ds:interval_per_tick, 3C75C28Fh         <-- float for 0.015
    unsigned char pattern[] = { 0xC7,0x05, '?','?','?','?', 0x8F,0xC2,0x75,0x3C, 0xE8 };
    void* addr = FindPattern(base, length, pattern, "xx????xxxxx");
    if (addr)
        interval_per_tick = *(float**)(addr + 2);

#elif defined (__APPLE__)

#endif

    return (interval_per_tick ? true : false);
}


bool TickSet::SetTickrate(int gameMode)
{
    ConVarRef customTickrate("sv_tickrate");

    switch (gameMode)
    {
    case MOMGM_BHOP:
    case MOMGM_SCROLL:
        //MOM_TODO: add more gamemodes
        return SetTickrate(s_DefinedRates[TICKRATE_100]);

    case MOMGM_UNKNOWN:
        return SetTickrate(customTickrate.GetFloat());

    case MOMGM_SURF:
    default:
        return SetTickrate(s_DefinedRates[TICKRATE_66]);
    }
}

bool TickSet::SetTickrate(float tickrate)
{
    if (!g_pMomentumUtil->FloatEquals(m_trCurrent.fTickRate, tickrate))
    {
        Tickrate tr;
        if (g_pMomentumUtil->FloatEquals(tickrate, 0.01f)) tr = s_DefinedRates[TICKRATE_100];
        else if (g_pMomentumUtil->FloatEquals(tickrate, 0.015f)) tr = s_DefinedRates[TICKRATE_66];
        else
        {
            tr.fTickRate = tickrate;
            tr.sType = "CUSTOM";
        }
        return SetTickrate(tr);
    }
    
    return false;
}

bool TickSet::SetTickrate(Tickrate trNew)
{
    if (trNew == m_trCurrent)
    {
        Msg("Tickrate not changed: new == current\n");
        return false;
    }

    if (interval_per_tick)
    {
        *interval_per_tick = trNew.fTickRate;
        gpGlobals->interval_per_tick = *interval_per_tick;
        m_trCurrent = trNew;
        for (int i = 0; i <= gpGlobals->maxClients; i++)
        {
            auto pPlayer = UTIL_PlayerByIndex(i);
            if (pPlayer)
            {
                engine->ClientCommand(pPlayer->edict(), "reload");
            }
        }

        Msg("Interval per tick set to %f\n", trNew.fTickRate);
        return true;
    }
    Msg("Failed to set tickrate: bad hook\n");
    return false;
}

static void onTickRateChange(IConVar *var, const char *pOldValue, float fOldValue)
{
    ConVarRef tr(var);
    float tickrate = tr.GetFloat();
    if (CloseEnough(tickrate, TickSet::GetTickrate(), FLT_EPSILON))
        return;
    // MOM_TODO: Re-implement the bound

    /*
    if (toCheck < 0.01f || toCheck > 0.015f)
    {
        Warning("Cannot set a tickrate any lower than 66 or higher than 100!\n");
        var->SetValue(((ConVar*) var)->GetDefault());
        return;
    }*/

    TickSet::SetTickrate(tickrate);
}


// MOM_TODO: Remove the comment in the flags
static ConVar tickRate("sv_tickrate", "0.015", FCVAR_CHEAT | FCVAR_REPLICATED /*| FCVAR_NOT_CONNECTED*/, "Changes the tickrate of the game.", onTickRateChange);