#include "c_mom_player.h"
#include "c_mapzones.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "cbase.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MapzoneData, DT_Mapzones, CMapzoneData)
    RecvPropString(RECVINFO(m_zones))
    END_RECV_TABLE();

#define NO_LOOK -190.0f

CMapzone::~CMapzone()
{
    if (m_pos)
    {
        delete m_pos;
        m_pos = nullptr;
    }
    if (m_rot)
    {
        delete m_rot;
        m_rot = nullptr;
    }
    if (m_scaleMins)
    {
        delete m_scaleMins;
        m_scaleMins = nullptr;
    }
    if (m_scaleMaxs)
    {
        delete m_scaleMaxs;
        m_scaleMaxs = nullptr;
    }
}

C_MapzoneData::C_MapzoneData() {}

CMapzone::CMapzone(const int pType, Vector *pPos, QAngle *pRot, Vector *pScaleMins, Vector *pScaleMaxs,
                   const int pIndex, const bool pShouldStop, const bool pShouldTilt, const float pHoldTime,
                   const bool pLimitSpeed, const float pBhopLeaveSpeed, const float flYaw, const string_t pLinkedEnt,
                   const bool pCheckOnlyXY, const int pBonusStage)
{
    m_type = pType;
    m_pos = pPos;
    m_rot = pRot;
    m_scaleMins = pScaleMins;
    m_scaleMaxs = pScaleMaxs;
    m_index = pIndex;
    m_shouldStopOnTeleport = pShouldStop;
    m_shouldResetAngles = pShouldTilt;
    m_holdTimeBeforeTeleport = pHoldTime;
    m_limitingspeed = pLimitSpeed;
    m_bhopleavespeed = pBhopLeaveSpeed;
    m_yaw = flYaw;
    m_linkedent = pLinkedEnt;
    m_onlyxycheck = pCheckOnlyXY;
    m_limitbhop = false;
    m_maxleavespeed = 0.0f;
    m_trigger = nullptr;
    m_bonusStage = pBonusStage;
}

C_MapzoneData::~C_MapzoneData()
{
    if (!m_zones.IsEmpty())
    {
        m_zones.PurgeAndDeleteElements();
    }
}

void C_MapzoneData::DrawMapZones()
{
    int MAX_RNG_VAL = 255;
    for (int i = 0; i < m_zones.Count(); i++)
    {
        CMapzone *zone = m_zones[i];

        int waveCycleVal = abs(int(sin(gpGlobals->curtime) * MAX_RNG_VAL));
        int RValue, GValue, BValue;
        RValue = GValue = BValue = waveCycleVal;

        Vector zonePosition = *zone->GetPosition();
        Vector zoneScaleMins = *zone->GetScaleMins();
        Vector zoneScaleMaxs = *zone->GetScaleMaxs();

        Vector TopLeftBack = {zonePosition.x + zoneScaleMaxs.x, zonePosition.y + zoneScaleMaxs.y,
                              zonePosition.z + zoneScaleMaxs.z};
        Vector TopRightBack = {zonePosition.x + zoneScaleMins.x, zonePosition.y + zoneScaleMaxs.y,
                               zonePosition.z + zoneScaleMaxs.z};
        Vector BottomLeftBack = {zonePosition.x + zoneScaleMaxs.x, zonePosition.y + zoneScaleMaxs.y,
                                 zonePosition.z + zoneScaleMins.z};
        Vector BottomRightBack = {zonePosition.x + zoneScaleMins.x, zonePosition.y + zoneScaleMaxs.y,
                                  zonePosition.z + zoneScaleMins.z};

        Vector TopLeftFront = {zonePosition.x + zoneScaleMaxs.x, zonePosition.y + zoneScaleMins.y,
                               zonePosition.z + zoneScaleMaxs.z};
        Vector TopRightFront = {zonePosition.x + zoneScaleMins.x, zonePosition.y + zoneScaleMins.y,
                                zonePosition.z + zoneScaleMaxs.z};
        Vector BottomLeftFront = {zonePosition.x + zoneScaleMaxs.x, zonePosition.y + zoneScaleMins.y,
                                  zonePosition.z + zoneScaleMins.z};
        Vector BottomRightFront = {zonePosition.x + zoneScaleMins.x, zonePosition.y + zoneScaleMins.y,
                                   zonePosition.z + zoneScaleMins.z};

        if (waveCycleVal > MAX_RNG_VAL / 3.5)
        {
            DebugDrawLine(TopLeftBack, TopRightBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(BottomLeftBack, BottomRightBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopLeftBack, BottomLeftBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopRightBack, BottomRightBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopLeftFront, TopRightFront, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(BottomLeftFront, BottomRightFront, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopLeftFront, BottomLeftFront, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopRightFront, BottomRightFront, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopLeftFront, TopLeftBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(BottomLeftFront, BottomLeftBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(TopRightFront, TopRightBack, RValue, GValue, BValue, false, -1.0f);
            DebugDrawLine(BottomRightFront, BottomRightBack, RValue, GValue, BValue, false, -1.0f);
        }
    }
}

C_MapzoneData g_Mapzones;