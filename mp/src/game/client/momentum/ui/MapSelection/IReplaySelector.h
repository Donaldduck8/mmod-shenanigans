#ifndef IREPLAYSELECTOR_H
#define IREPLAYSELECTOR_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui/IVGui.h"

class IReplaySelector
{
public:
    virtual void Create(vgui::VPANEL) = 0;
    virtual void Destroy() = 0;
    virtual void Activate() = 0;
    virtual void Deactivate() = 0;
};

extern IReplaySelector* replayselector;

#endif