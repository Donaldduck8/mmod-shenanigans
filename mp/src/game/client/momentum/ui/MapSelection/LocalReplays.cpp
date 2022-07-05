#include "pch_mapselection.h"

using namespace vgui;

extern IFileSystem *filesystem;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLocalReplays::CLocalReplays(vgui::Panel *parent) :
CBaseReplaysPage(parent, "LocalReplays")
{
    m_bLoadedMaps = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLocalReplays::~CLocalReplays()
{
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CLocalReplays::OnPageShow()
{
    if (!m_bLoadedMaps)
        GetNewMapList();

    StartRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CLocalReplays::SupportsItem(InterfaceItem_e item)
{
    switch (item)
    {
    case FILTERS:
        return true;

    case GETNEWLIST:
    default:
        return false;
    }
}


bool CLocalReplays::MapHasStages(const char* szMap)
{
    bool found = false;
    if (filesystem && szMap)
    {
        KeyValues *kvMap = new KeyValues(szMap);
        char path[MAX_PATH];
        char fileName[FILENAME_MAX];
        Q_snprintf(fileName, FILENAME_MAX, "%s%s", szMap, EXT_ZONE_FILE);
        V_ComposeFileName(MAP_FOLDER, fileName, path, MAX_PATH);


        if (kvMap->LoadFromFile(filesystem, path, "MOD"))
        {
            found = (kvMap->FindKey("stage") != nullptr);
        }
        kvMap->deleteThis();
    }

    return found;
}

void CLocalReplays::FillMapstruct(replaystruct_t *m)
{
    //Game mode
    m->m_iGameMode = MOMGM_UNKNOWN;
    float tickRate = 0.015f;
    if (!Q_strnicmp(m->m_szMapName, "surf_", strlen("surf_")))
    {
        m->m_iGameMode = MOMGM_SURF;
    }
    else if (!Q_strnicmp(m->m_szMapName, "bhop_", strlen("bhop_")) || !Q_strnicmp(m->m_szMapName, "trikz_", strlen("trikz_")))
    {
        m->m_iGameMode = MOMGM_BHOP;
        tickRate = 0.01f;
    }

    // MOM_TODO: Determine difficulty
    m->m_iDifficulty = 1;

    //Map layout (liner/staged)
    m->m_bHasStages = MapHasStages(m->m_szMapName);

    //Completed/Best time
    //MOM_TODO: have the tickrate and run flags as filters, load actual values
    
    CMomReplayBase *pBestTime = g_pMomentumUtil->GetBestTime(m->m_szMapName, tickRate);
    if (pBestTime)
    {
        m->m_bCompleted = true;
        g_pMomentumUtil->FormatTime(pBestTime->GetRunTime(), m->m_szBestTime);
    }
}

// Gross hack needed because scheme()->GetImage still returns an image even if it's null (returns the null texture)
inline bool ImageExists(const char *pMapName)
{
    FileFindHandle_t found;
    const char *pStr = g_pFullFileSystem->FindFirstEx(VarArgs("materials/vgui/maps/%s.vmt", pMapName), "GAME", &found);
    return pStr ? true : false;
}

inline bool DemoExists(const char *pMapName)
{
    FileFindHandle_t found;
    char path[MAX_PATH];
    Q_snprintf(path, MAX_PATH, "%s/%s*%s", RECORDING_PATH, pMapName, EXT_RECORDING_FILE);
    V_FixSlashes(path);

    const char *pFoundFile = filesystem->FindFirstEx(path, "MOD", &found);
    return pFoundFile ? true : false;
}

void CLocalReplays::GetNewMapList()
{
    ClearMapList();
    //Populate the main list
    FileFindHandle_t found;

    const char *pMapName = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "GAME", &found);
    while (pMapName)
    {   
        replaystruct_t m = replaystruct_t();
        Q_FileBase(pMapName, m.m_szMapName, MAX_PATH);

        // If a demo exists for a given map, add it to the replays list
        if (DemoExists(m.m_szMapName))
        {
            FillMapstruct(&m);
            replaydisplay_t map = replaydisplay_t();
            map.m_bDoNotRefresh = true;
            map.m_mMap = m;

            // Map image
            if (ImageExists(m.m_szMapName))
            {
                DevLog("FOUND IMAGE FOR %s!\n", m.m_szMapName);
                char imagePath[MAX_PATH];
                Q_snprintf(imagePath, MAX_PATH, "maps/%s", m.m_szMapName);
                map.m_iMapImageIndex = m_pMapList->GetImageList()->AddImage(scheme()->GetImage(imagePath, false));
            }

            m_vecReplays.AddToTail(map);
        }
        pMapName = g_pFullFileSystem->FindNext(found);
    }
    g_pFullFileSystem->FindClose(found);

    ApplyGameFilters();
}

//-----------------------------------------------------------------------------
// Purpose: starts the maps refreshing
//-----------------------------------------------------------------------------
void CLocalReplays::StartRefresh()
{
    FOR_EACH_VEC(m_vecReplays, i)
    {
        replaydisplay_t *pMap = &m_vecReplays[i];
        if (!pMap) continue;
        replaystruct_t pMapInfo = pMap->m_mMap;
        // check filters
        bool removeItem = false;
        if (!CheckPrimaryFilters(pMapInfo))
        {
            // map has been filtered at a primary level
            // remove from lists
            pMap->m_bDoNotRefresh = true;

            // remove from UI list
            removeItem = true;
        }
        else if (!CheckSecondaryFilters(pMapInfo))
        {
            // we still ping this server in the future; however it is removed from UI list
            removeItem = true;
        }

        if (removeItem)
        {
            if (m_pMapList->IsValidItemID(pMap->m_iListID))
            {
                m_pMapList->RemoveItem(pMap->m_iListID);
                pMap->m_iListID = GetInvalidMapListID();
            }
            return;
        }

        // update UI
        KeyValues *kv;
        if (m_pMapList->IsValidItemID(pMap->m_iListID))
        {
            // we're updating an existing entry
            kv = m_pMapList->GetItem(pMap->m_iListID);
        }
        else
        {
            // new entry
            kv = new KeyValues("Map");
        }
        
        kv->SetString(KEYNAME_MAP_NAME, pMapInfo.m_szMapName);
        kv->SetString(KEYNAME_MAP_LAYOUT, pMapInfo.m_bHasStages ? "STAGED" : "LINEAR");
        kv->SetInt(KEYNAME_MAP_DIFFICULTY, pMapInfo.m_iDifficulty);
        kv->SetString(KEYNAME_MAP_BEST_TIME, pMapInfo.m_szBestTime);
        kv->SetInt(KEYNAME_MAP_IMAGE, pMap->m_iMapImageIndex);
        
        if (!m_pMapList->IsValidItemID(pMap->m_iListID))
        {
            // new map, add to list
            pMap->m_iListID = m_pMapList->AddItem(kv, NULL, false, false);
            if (m_bAutoSelectFirstItemInGameList && m_pMapList->GetItemCount() == 1)
            {
                m_pMapList->AddSelectedItem(pMap->m_iListID);
            }

            kv->deleteThis();
        }
        else
        {
            // tell the list that we've changed the data
            m_pMapList->ApplyItemChanges(pMap->m_iListID);
            m_pMapList->SetItemVisible(pMap->m_iListID, true);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose: Control which button are visible.
//-----------------------------------------------------------------------------
void CLocalReplays::ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter)
{
    m_pStartMap->SetVisible(bShowConnect);
    m_pRefreshAll->SetVisible(bShowRefreshAll);
    m_pFilter->SetVisible(bShowFilter);
}

void CLocalReplays::SetEmptyListText()
{
    m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a map)
//-----------------------------------------------------------------------------
void CLocalReplays::OnOpenContextMenu(int row)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pMapList);
    menu->ShowMenu(this, true, true);
}