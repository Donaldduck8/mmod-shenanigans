#include "pch_mapselection.h"

static CReplaySelector g_ReplaySelectorPanel;
IReplaySelector *replayselector = static_cast<CReplaySelector *>(&g_ReplaySelectorPanel);

//
CReplaySelector::CReplaySelector()
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CReplaySelector::~CReplaySelector()
{
}

CON_COMMAND_F(ShowReplaySelectionPanel, "Shows ReplaySelectorPanel", FCVAR_CLIENTDLL | FCVAR_HIDDEN)
{
    replayselector->Activate();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CReplaySelector::Create(vgui::VPANEL parent)
{
    if (!m_hReplaysDlg.Get())
    {
        m_hReplaysDlg = new CReplaySelectorDialog(parent); // SetParent() call below fills this in
        m_hReplaysDlg->Initialize();
    }
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CReplaySelector::Activate()
{
    static bool m_bfirstTimeOpening = true;
    if (m_bfirstTimeOpening)
    {
        m_hReplaysDlg->LoadUserData(); // reload the user data the first time the dialog is made visible, 
        //helps with the lag between module load and steamui getting Deactivate() call
        m_bfirstTimeOpening = false;
    }

    Open();
}

//-----------------------------------------------------------------------------
// Purpose: called when the server browser gets closed by the enduser
//-----------------------------------------------------------------------------
void CReplaySelector::Deactivate()
{
    if (m_hReplaysDlg.Get())
    {
        m_hReplaysDlg->SaveUserData();
        m_hReplaysDlg->Close();
        CloseAllMapInfoDialogs();
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CReplaySelector::Open()
{
    m_hReplaysDlg->Open();
}

//-----------------------------------------------------------------------------
// Purpose: Closes down the server browser for good
//-----------------------------------------------------------------------------
void CReplaySelector::Destroy()
{
    if (m_hReplaysDlg.Get())
    {
        m_hReplaysDlg->Close();
        m_hReplaysDlg->MarkForDeletion();
        m_hReplaysDlg = nullptr;
    }
}

//-----------------------------------------------------------------------------
// Purpose: closes all the game info dialogs
//-----------------------------------------------------------------------------
void CReplaySelector::CloseAllMapInfoDialogs()
{
    if (m_hReplaysDlg.Get())
    {
        m_hReplaysDlg->CloseAllMapInfoDialogs();
    }
}
