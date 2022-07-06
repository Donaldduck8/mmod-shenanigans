#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "momentum/tickset.h"
#include "KeyValues.h"
#include "momentum/util/mom_util.h"
#include "filesystem.h"
#include "mom_triggers.h"
#include "GameEventListener.h"
#include "tier1/checksum_sha1.h"
#include "momentum/mom_shareddefs.h"
#include "momentum/mom_gamerules.h"
#include "mom_replay_system.h"
#include "movevars_shared.h"
#include <ctime>
#include <map>

class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;
class CTriggerStage;

class CTimerData
{
public:
	// int m_iZoneCount;
	int m_iStartTick, m_iEndTick = 0;
	int m_iLastZone = 0;
	time_t m_iLastRunDate = 0;
	bool m_bIsRunning = false;

	Checkpoint *m_pStartZoneMark = nullptr;

	CHandle<CTriggerCheckpoint> m_pCurrentCheckpoint = nullptr;
	CHandle<CTriggerStage> m_pCurrentZone = nullptr; // MOM_TODO: Change to be the generic Zone trigger

	// PRECISION FIX:
	// this works by adding the starting offset to the final time, since the timer starts after we actually exit the
	// start trigger
	// also, subtract the ending offset from the time, since we end after we actually enter the ending trigger
	float m_flTickOffsetFix[MAX_STAGES]; // index 0 = endzone, 1 = startzone, 2 = stage 2, 3 = stage3, etc
	float m_flZoneEnterTime[MAX_STAGES];

	float m_flDistFixTraceCorners[8]; // array of floats representing the trace distance from each corner of the
									  // player's collision hull

};

class CMomentumTimer : CAutoGameSystem
{
  public:
      CMomentumTimer(const char *pName)
        : CAutoGameSystem(pName), m_iZoneCount(0), /*m_iStartTick(0), m_iEndTick(0), m_iLastZone(0), m_iLastRunDate(0), m_bIsRunning(false)* / , m_bWereCheatsActivated(false), /*m_bMapIsLinear(false), */ m_pStartTrigger(nullptr),
          m_pEndTrigger(nullptr) /*m_pCurrentCheckpoint(nullptr), m_pCurrentZone(nullptr), m_pLocalTimes(nullptr),*/
          /*m_pStartZoneMark(nullptr)*/
    {
    }

	CTimerData* GetTimerDataForPlayer(CMomentumPlayer* pPlayer);

	std::map<uint64, CTimerData*> m_pPlayersTimerData;

    //-------- HUD Messages --------------------
    void DispatchResetMessage(CMomentumPlayer *pPlayer);
    //Plays the hud_timer effects to a specific player
    void DispatchTimerStateMessage(CBasePlayer* pPlayer, bool isRunning) const;

    // ------------- Timer state related messages --------------------------
    // Strats the timer for the given starting tick
    void Start(CMomentumPlayer *pPlayer, int startTick);
    // Stops the timer
    void Stop(CMomentumPlayer* pPlayer, bool endTrigger);
    // Is the timer running?
    bool IsRunning(CMomentumPlayer *pPlayer) {
		// TRIKZ NETWORK TIMERS: HELP
		if (!pPlayer)
			return false;
		return GetTimerDataForPlayer(pPlayer)->m_bIsRunning;
	}
    // Set the running status of the timer
    void SetRunning(CMomentumPlayer *pPlayer, bool running);

    // ------------- Timer trigger related methods ----------------------------
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger() const { return m_pStartTrigger.Get(); }
    // Gets the current checkpoint
    CTriggerCheckpoint *GetCurrentCheckpoint(CMomentumPlayer *pPlayer) {
		// TRIKZ NETWORK TIMERS: HELP
		if (!pPlayer)
			return new CTriggerCheckpoint();
		return GetTimerDataForPlayer(pPlayer)->m_pCurrentCheckpoint.Get();
	}

    CTriggerTimerStop *GetEndTrigger() const { return m_pEndTrigger.Get(); }
    CTriggerStage *GetCurrentStage(CMomentumPlayer* pPlayer) {
		// TRIKZ NETWORK TIMERS: HELP
		if (!pPlayer)
			return new CTriggerStage();
		return GetTimerDataForPlayer(pPlayer)->m_pCurrentZone.Get();
	}

    // Sets the given trigger as the start trigger
    // TRIKZ NETWORK TIMERS: Not changed, giving it the benefit of the doubt
    void SetStartTrigger(CTriggerTimerStart *pTrigger)
    {
		for (int i = 0; i < gpGlobals->maxClients; i++) {
			CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_PlayerByIndex(i));
			
			if (pPlayer)
				GetTimerDataForPlayer(pPlayer)->m_iLastZone = 0; // Allows us to overwrite previous runs
		}
        m_pStartTrigger.Set(pTrigger);
    }

    // Sets the current checkpoint
    void SetCurrentCheckpointTrigger(CMomentumPlayer *pPlayer, CTriggerCheckpoint *pTrigger) {
		if (pPlayer)
			GetTimerDataForPlayer(pPlayer)->m_pCurrentCheckpoint.Set(pTrigger);
	}

    void SetEndTrigger(CTriggerTimerStop *pTrigger) { m_pEndTrigger.Set(pTrigger); }
    //MOM_TODO: Change this to be the CTriggerZone class
    void SetCurrentZone(CMomentumPlayer* pPlayer, CTriggerStage *pTrigger)
    {
		if (pPlayer)
			GetTimerDataForPlayer(pPlayer)->m_pCurrentZone.Set(pTrigger);
    }
    int GetCurrentZoneNumber(CMomentumPlayer* pPlayer) {
		if (!pPlayer)
			return 0;
		return GetTimerDataForPlayer(pPlayer)->m_pCurrentZone.Get() && GetTimerDataForPlayer(pPlayer)->m_pCurrentZone.Get()->GetStageNumber();
	}

    // Calculates the stage count
    // Stores the result on m_iStageCount
    void RequestZoneCount();
    // Gets the total stage count
    int GetZoneCount() const { return m_iZoneCount; };
    float CalculateStageTime(CMomentumPlayer *pPlayer, int stageNum);
    // Gets the time for the last run, if there was one
    float GetLastRunTime(CMomentumPlayer *pPlayer)
    {
		if (!pPlayer)
			return 0.0f;

		CTimerData *pData = GetTimerDataForPlayer(pPlayer);
        if (pData->m_iEndTick == 0)
            return 0.0f;
        float originalTime = static_cast<float>(pData->m_iEndTick - pData->m_iStartTick) * gpGlobals->interval_per_tick;
        // apply precision fix, adding offset from start as well as subtracting offset from end.
        // offset from end is 1 tick - fraction offset, since we started trace outside of the end zone.
        return originalTime + pData->m_flTickOffsetFix[1] - (gpGlobals->interval_per_tick - pData->m_flTickOffsetFix[0]);
    }
    // Gets the date achieved for the last run.
    time_t GetLastRunDate(CMomentumPlayer *pPlayer)
    {
		if (!pPlayer)
			return 0;
        return GetTimerDataForPlayer(pPlayer)->m_iLastRunDate;
    }

    // Gets the current time for this timer
    float GetCurrentTime(CMomentumPlayer* pPlayer) {
		CTimerData* pData = GetTimerDataForPlayer(pPlayer);
		return float(gpGlobals->tickcount - pData->m_iStartTick) * gpGlobals->interval_per_tick;
	}

    //----- Trigger_Onehop stuff -----------------------------------------
    // Removes the given Onehop form the hopped list.
    // Returns: True if deleted, False if not found.
    bool RemoveOnehopFromList(CTriggerOnehop *pTrigger);
    // Adds the give Onehop to the hopped list.
    // Returns: Its new index.
    int AddOnehopToListTail(CTriggerOnehop *pTrigger);
    // Finds a Onehop on the hopped list.
    // Returns: Its index. -1 if not found
    int FindOnehopOnList(CTriggerOnehop *pTrigger);
    // Removes all onehops from the list
    void RemoveAllOnehopsFromList() { onehops.RemoveAll(); }
    // Returns the count for the onehop list
    int GetOnehopListCount() const { return onehops.Count(); }
    // Finds the onehop with the given index on the list
    CTriggerOnehop *FindOnehopOnList(int pIndexOnList);

    //-------- Online-related timer commands -----------------------------
    // MOM_TODO: void LoadOnlineTimes();

    // Level init/shutdown hooks
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void DispatchMapInfo() const;

    // Practice mode- noclip mode that stops timer
    void EnablePractice(CMomentumPlayer *pPlayer);
    void DisablePractice(CMomentumPlayer *pPlayer);

    // Have the cheats been turned on in this session?
    bool GotCaughtCheating() const { return m_bWereCheatsActivated; };
    void SetCheating(bool newBool)
    {
        UTIL_ShowMessage("CHEATER", UTIL_GetLocalPlayer());
		for (int i = 0; i < gpGlobals->maxClients; i++) {
			CMomentumPlayer* pPlayer = ToCMOMPlayer(UTIL_PlayerByIndex(i));
			if (pPlayer)
				Stop(pPlayer, false);

		}

        m_bWereCheatsActivated = newBool;
    }

    void SetGameModeConVars();

    void CreateStartMark(CMomentumPlayer *pPlayer);
    Checkpoint *GetStartMark(CMomentumPlayer* pPlayer) {
		if (!pPlayer) {
			return new Checkpoint();
		}
		return GetTimerDataForPlayer(pPlayer)->m_pStartZoneMark;
	}
    void ClearStartMark(CMomentumPlayer* pPlayer);

  private:
    int m_iZoneCount;
    bool m_bWereCheatsActivated;
    bool m_bMapIsLinear;

    CHandle<CTriggerTimerStart> m_pStartTrigger;
    CHandle<CTriggerTimerStop> m_pEndTrigger;

    CUtlVector<CTriggerOnehop *> onehops;
    KeyValues *m_pLocalTimes;

public:
    // creates fraction of a tick to be used as a time "offset" in precicely calculating the real run time.
    void CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, const int zoneType);
    void SetIntervalOffset(CMomentumPlayer* pPlayer, int stage, float offset) { 
		GetTimerDataForPlayer(pPlayer)->m_flTickOffsetFix[stage] = offset;
	}
    typedef enum { ZONETYPE_END, ZONETYPE_START } zoneType;
};



class CTimeTriggerTraceEnum : public IEntityEnumerator
{
  public:
    CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity, int zoneType, int cornerNum)
        : m_iZoneType(zoneType), m_iCornerNumber(cornerNum), m_pRay(pRay)
    {
    }

    bool EnumEntity(IHandleEntity *pHandleEntity) OVERRIDE;

  private:
    int m_iZoneType;
    int m_iCornerNumber;
    Ray_t *m_pRay;
};

extern CMomentumTimer *g_pMomentumTimer;

#endif // TIMER_H