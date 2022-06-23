#include "cbase.h"
#include "mom_entity_run_data.h"
#include "tier0/memdbgon.h"
#include "mom_player_shared.h"

#if !defined(CLIENT_DLL)
#include "sendproxy.h"
#endif

#include "../../../public/dt_send.h"


#ifdef CLIENT_DLL
BEGIN_RECV_TABLE_NOBASE(C_MOMRunEntityData, DT_MOM_RunEntData)
RecvPropBool(RECVINFO(m_bAutoBhop)),
RecvPropInt(RECVINFO(m_iSuccessiveBhops)),
RecvPropFloat(RECVINFO(m_flStrafeSync)),
RecvPropFloat(RECVINFO(m_flStrafeSync2)),
RecvPropFloat(RECVINFO(m_flLastJumpVel)),
RecvPropFloat(RECVINFO(m_flLastJumpTime)),
RecvPropInt(RECVINFO(m_iRunFlags)),
RecvPropBool(RECVINFO(m_bIsInZone)),
RecvPropInt(RECVINFO(m_iCurrentZone)),
RecvPropBool(RECVINFO(m_bMapFinished)),
RecvPropBool(RECVINFO(m_bTimerRunning)),
RecvPropInt(RECVINFO(m_iStartTick)), 
RecvPropInt(RECVINFO(m_iEndTick)),
RecvPropBool(RECVINFO(m_bIsRunning)), 
RecvPropInt(RECVINFO(m_iStartTickD)),
RecvPropFloat(RECVINFO(m_flRunTime)),
RecvPropInt(RECVINFO(m_iPlayerIndex)),
END_RECV_TABLE()
BEGIN_RECV_TABLE_NOBASE(CMOMRunEntityData, DT_MOM_RunEntData_Parent) 
RecvPropDataTable("m_RunData", 0, 0, &REFERENCE_RECV_TABLE(DT_MOM_RunEntData)) 
END_RECV_TABLE()
#else
void *SendProxy_SendRunEntData(const SendProp *pProp, const void *pStruct, const void *pVarData,
                               CSendProxyRecipients *pRecipients, int objectID)
{
    CMOMRunEntityData *pPlayer = dynamic_cast<CMOMRunEntityData *>((CMOMRunEntityData *)pStruct);

    pRecipients->SetOnly(pPlayer->m_iPlayerIndex);
    Msg("SendRunEntData Proxy: %i\n", pPlayer->m_iPlayerIndex);

    return (void *)pVarData;
}


BEGIN_SEND_TABLE_NOBASE(CMOMRunEntityData, DT_MOM_RunEntData)
SendPropBool(SENDINFO(m_bAutoBhop)),
SendPropInt(SENDINFO(m_iSuccessiveBhops)),
SendPropFloat(SENDINFO(m_flStrafeSync)),
SendPropFloat(SENDINFO(m_flStrafeSync2)),
SendPropFloat(SENDINFO(m_flLastJumpVel)),
SendPropFloat(SENDINFO(m_flLastJumpTime)),
SendPropInt(SENDINFO(m_iRunFlags)),
SendPropBool(SENDINFO(m_bIsInZone)),
SendPropInt(SENDINFO(m_iCurrentZone)),
SendPropInt(SENDINFO(m_bMapFinished), 1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN),
SendPropBool(SENDINFO(m_bTimerRunning)),
SendPropInt(SENDINFO(m_iStartTick)), 
SendPropInt(SENDINFO(m_iEndTick)),
SendPropBool(SENDINFO(m_bIsRunning)),
SendPropInt(SENDINFO(m_iStartTickD)),
SendPropFloat(SENDINFO(m_flRunTime)),
SendPropInt(SENDINFO(m_iPlayerIndex)),
END_SEND_TABLE()
BEGIN_SEND_TABLE_NOBASE(CMOMRunEntityData, DT_MOM_RunEntData_Parent) 
//SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData_Parent),
 //                         CMomentumPlayer::SendProxy_SendRunEntData),
//SendPropDataTable("RunData", 0, &DT_MOM_RunEntData::g_SendTable, SendProxy_SendRunEntData) 
SendPropDataTable("m_RunData", 0, &REFERENCE_SEND_TABLE(DT_MOM_RunEntData), SendProxy_SendRunEntData)
END_SEND_TABLE() 
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendRunEntData);
#endif

CMOMRunEntityData::CMOMRunEntityData()
{
    m_bAutoBhop = false;
    m_iSuccessiveBhops = 0;
    m_flStrafeSync = 0.0f;
    m_flStrafeSync2 = 0.0f;
    m_flLastJumpVel = 0.0f;
    m_flLastJumpTime = 0.0f;
    m_iRunFlags = 0;
    m_bIsInZone = false;
    m_iCurrentZone = 0;
    m_iStartTick = -1;
	m_iStartTickD = -1;
    m_bMapFinished = false;
    m_bTimerRunning = false;
    m_flRunTime = 0.0f;
    m_iPlayerIndex = -1;
}

