#ifndef CLIENT_EVENTS_H
#define CLIENT_EVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class CMOMClientEvents : public CAutoGameSystemPerFrame
{
public:
    CMOMClientEvents(const char *pName) : CAutoGameSystemPerFrame(pName)
    {
    }

    //After DLL inits successfully
    void PostInit() OVERRIDE;

    void LevelInitPostEntity() OVERRIDE;

    void Update(float frametime) OVERRIDE;

    //From the Valve SDK wiki
    static void MountAdditionalContent();

};

#endif // CLIENT_EVENTS_H