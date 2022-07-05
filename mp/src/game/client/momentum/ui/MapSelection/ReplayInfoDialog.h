#ifndef DIALOGREPLAYINFO_H
#define DIALOGREPLAYINFO_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying information about a game server
//-----------------------------------------------------------------------------
class CDialogReplayInfo : public Frame
{
    DECLARE_CLASS_SIMPLE(CDialogReplayInfo, vgui::Frame);

public:
    CDialogReplayInfo(Panel *parent, const char*);
    ~CDialogReplayInfo();

    void Run(const char *titleName);

    // forces the dialog to attempt to connect to the server
    void Connect();

    // on individual player added
    virtual void AddPlayerToList(KeyValues *pPlayerInfo);

    // called when the current refresh list is complete
    virtual void RefreshComplete(EMatchMakingServerResponse response);

    // player list received
    virtual void ClearPlayerList();

protected:
    // message handlers
    MESSAGE_FUNC(OnConnect, "Connect");
    // vgui overrides
    void OnTick() OVERRIDE;
    void PerformLayout() OVERRIDE;

    // API
    void GetMapInfo(const char* mapname);
    void GetMapInfoCallback(HTTPRequestCompleted_t*, bool);
    CCallResult<CDialogReplayInfo, HTTPRequestCompleted_t> cbGetMapInfoCallback;

    void Get10MapTimes(const char *mapname);
    void GetReplayInfo(const char *mapname);
    void Get10MapTimesCallback(HTTPRequestCompleted_t*, bool);
    CCallResult<CDialogReplayInfo, HTTPRequestCompleted_t> cbGet10MapTimesCallback;


private:

    static int PlayerTimeColumnSortFunc(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2);

    // methods
    void RequestInfo(const char* mapName);
    void ConnectToServer();
    void ApplyConnectCommand(const char *mapName);

    Button *m_pConnectButton;
    Button *m_pCloseButton;
    ListPanel *m_pPlayerList;

    enum { PING_TIMES_MAX = 4 };

    // true if we should try connect to the server when it refreshes
    bool m_bConnecting;
    bool m_bPlayerListUpdatePending;
};

#endif // DIALOGGAMEINFO_H
