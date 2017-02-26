/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PcapTask_t.h
//
// Packet capture task (Acquire Handler).
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_PCAPTASK_T_H_
#define Include_PCAPTASK_T_H_

#include "DpHandler_t.h"
//#include "DpEvent_t.h"
#include "AutoHand.h"

/////////////////////////////////////////////////////////////////////////////

class PipelineManager_t;
struct pcap;

class PcapTask_t :
    public DP::Handler_t
{

private:

	CAutoHandle m_hThread;
	CAutoHandle m_hExitEvent;

    DWORD  m_dwThreadId;
    pcap*  m_fp;

public:

    PcapTask_t();

	virtual
    ~PcapTask_t();

    //
    // DP::Handler_t virtual
    //

    virtual
    bool
    Initialize(
       const wchar_t* pszClass);

    virtual
    HRESULT
    EventHandler(
        DP::Event::Data_t& Data);

    //
    // PcapTask virtual:
    //

    virtual
    void
    PostData(
        const unsigned char* pData);

private:

    bool
    Start();

    void
    Stop();

    bool
    ReadIniFile(
        const std::string& strFile,
             std::string& strDevice,
             std::string& strFilter);

    bool
    InitPcap(
        const std::string& strDevice,
        const std::string& strFilter);

	bool
    SameThread() const
    {
        return GetCurrentThreadId() == m_dwThreadId;
    }
	
    bool
    CheckPacket(
        const unsigned char* pkt_data);

    const char *
    GetErrorString() const;

private:

	static
    DWORD WINAPI
    ThreadFunc(void *pvParam);

private:

    PcapTask_t(const PcapTask_t&);
    PcapTask_t& operator=(const PcapTask_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_PCAPTASK_T_H_

/////////////////////////////////////////////////////////////////////////////
