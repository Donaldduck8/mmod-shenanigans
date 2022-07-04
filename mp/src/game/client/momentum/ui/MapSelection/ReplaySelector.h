#ifndef REPLAYSELECTOR_H
#define REPLAYSELECTOR_H

#ifdef _WIN32
#pragma once
#endif

class CReplaySelectorDialog;

class CReplaySelector : public IReplaySelector
{
public:
    CReplaySelector();
    ~CReplaySelector();

    void Create(VPANEL parent) OVERRIDE;
    void Destroy() OVERRIDE;
    void Activate() OVERRIDE;
    void Deactivate() OVERRIDE;

    void Open();
    void CloseAllMapInfoDialogs();

private:
    DHANDLE<CReplaySelectorDialog> m_hReplaysDlg;
};

extern IReplaySelector *replayselector;

#endif
