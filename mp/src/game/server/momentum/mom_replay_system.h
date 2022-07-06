#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H

#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "run/mom_replay_data.h"
#include <momentum/mom_player_shared.h>
#include "run/mom_replay_manager.h"

class CMomentumReplayGhostEntity;

class CMomentumReplaySystem : public CAutoGameSystemPerFrame
{
public:
    CMomentumReplaySystem(const char *pName) :
        CAutoGameSystemPerFrame(pName),
        m_bShouldStopRec(false),
        m_iTickCount(0),
        m_iStartRecordingTick(-1),
        m_iStartTimerTick(-1),
        m_fRecEndTime(-1.0f),
        m_player(nullptr)
    {
        m_pReplayManager = new CMomReplayManager();
    }

    virtual ~CMomentumReplaySystem() OVERRIDE
    {
        delete m_pReplayManager;
    }

public:
    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() OVERRIDE
    {
        if (m_pReplayManager->Recording())
            UpdateRecordingParams();
    }

    void LevelShutdownPostEntity() OVERRIDE
    {
        //Stop a recording if there is one while the level shuts down
		for (int i = 0; i < gpGlobals->maxClients; i++) {
			CMomentumPlayer* pPlayer = ToCMOMPlayer(UTIL_PlayerByIndex(i));

			if (!pPlayer)
				continue;

			if (m_pReplayManager->Recording())
				StopRecording(pPlayer, true, false);

			if (m_pReplayManager->GetPlaybackReplay())
				m_pReplayManager->UnloadPlayback(true);
		}
    }

    //Sets the start timer tick, this is used for trimming later on
    void SetTimerStartTick(int tick)
    {
        m_iStartTimerTick = tick;
    }

    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CMomentumPlayer* pPlayer, bool throwaway, bool delay);
    void TrimReplay(); //Trims a replay's start down to only include a defined amount of time in the start trigger

    inline CMomReplayManager* GetReplayManager() const { return m_pReplayManager; }

private:
    void UpdateRecordingParams(); // called every game frame after entities think and update
    void SetReplayInfo(CMomentumPlayer *pPlayer);
    void SetRunStats();

    bool m_bShouldStopRec;
    int m_iTickCount;// MOM_TODO: Maybe remove me?
    int m_iStartRecordingTick;//The tick that the replay started, used for trimming.
    int m_iStartTimerTick;//The tick that the player's timer starts, used for trimming.
    float m_fRecEndTime;// The time to end the recording, if delay was passed as true to StopRecording()

    CMomentumPlayer *m_player;
    CMomReplayManager* m_pReplayManager;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif // MOM_REPLAY_H
