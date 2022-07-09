#ifndef C_MAPZONES_H
#define C_MAPZONES_H
#ifdef WIN32
#pragma once
#endif

#include "c_mom_replay_entity.h"
#include "cbase.h"
#include <momentum/mom_shareddefs.h>
#include <run/mom_entity_run_data.h>
#include <run/run_stats.h>
#include "filesystem.h"

class C_Mapzones
{
  public:
    DECLARE_CLIENTCLASS();

};

class CMapzone
{
  public:
    CMapzone(const int, Vector *, QAngle *, Vector *, Vector *, const int, const bool, const bool, const float,
             const bool, const float, const float, const string_t, const bool, const int);
    ~CMapzone();

    Vector *GetPosition() { return m_pos; }
    QAngle *GetRotation() { return m_rot; }
    Vector *GetScaleMins() { return m_scaleMins; }
    Vector *GetScaleMaxs() { return m_scaleMaxs; }

  private:
    int m_type;                     // Zone type, look above
    int m_index;                    // Ignored when not a checkpoint
    bool m_shouldStopOnTeleport;    // Stop player on teleport?
    bool m_shouldResetAngles;       // Reset the player's angles?
    float m_holdTimeBeforeTeleport; // How much to wait for before teleporting
    // startTrigger
    bool m_limitingspeed; // Limit leave speed?
    bool m_onlyxycheck;   // Only checking speed in XY?
    bool m_limitbhop;
    float m_maxleavespeed;  // Max speed allowed
    float m_bhopleavespeed; // Max speed if player bhopped
    float m_yaw;            // Teleport yaw for start zone.
    string_t m_linkedent;   // Entity name for teleporting to this entity (YESYES, It can be null!)
    Vector *m_pos;
    QAngle *m_rot;
    Vector *m_scaleMins;
    Vector *m_scaleMaxs;
    CBaseEntity *m_trigger;
    int m_bonusStage;
};

class C_MapzoneData : public C_BaseEntity
{
  public:
    DECLARE_CLASS(C_MapzoneData, C_BaseEntity);
    DECLARE_CLIENTCLASS();
    void DrawMapZones();
    C_MapzoneData();
    ~C_MapzoneData();

    CUtlVector<CMapzone *> m_zones;
};

extern C_MapzoneData g_Mapzones;
#endif