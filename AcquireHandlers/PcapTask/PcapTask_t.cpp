///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PcapTask_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define HAVE_REMOTE
#include <Pcap.h>
#include "PipelineManager.h"
#include "PcapTask_t.h"
#include "Log.h"
#include "Macros.h"

//#define FAST

///////////////////////////////////////////////////////////////////////////////

PcapTask_t::
PcapTask_t() :
    m_fp(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////

PcapTask_t::
~PcapTask_t()
{
 	if (NULL != m_hThread.get())
        Stop();
}

///////////////////////////////////////////////////////////////////////////////

bool
PcapTask_t::
Initialize(
    const wchar_t* pszClass)
{
    LogInfo(L"PcapTask_t::Initialize()");

    if (!Handler_t::Initialize(pszClass))
        return false;

    std::string strDevice;
    std::string strFilter;
    if (!ReadIniFile(std::string("pcap.ini"), strDevice, strFilter))
        return false;

    if (!InitPcap(strDevice, strFilter))
        return false;

	m_hExitEvent = CreateEvent(0, TRUE, FALSE, 0);
    if (NULL == m_hExitEvent.get())
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
PcapTask_t::
ReadIniFile(
    const std::string& strFile,
          std::string& strDevice,
          std::string& strFilter)
{
    std::ifstream Fstream(strFile.c_str());
    if (Fstream.fail() || !Fstream.is_open())
    {
        LogError(L"ReadIniFile(%hs): Open failed.", strFile.c_str());
        return false;
    }
    char buf[256];
    Fstream.getline(buf, _countof(buf));
    if ('\0' == buf[0])
    {
        LogError(L"ReadIniFile: Read device failed.");
        return false;
    }
    strDevice = buf;
    LogAlways(L"Pcap: Device = %hs", strDevice.c_str());

    Fstream.getline(buf, _countof(buf));
    if ('\0' == buf[0])
    {
        LogError(L"ReadIniFile: Read filter failed.");
        return false;
    }
    strFilter = buf;
    LogAlways(L"Pcap: Filter = %hs", strFilter.c_str());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
PcapTask_t::
InitPcap(
    const std::string& strDevice,
    const std::string& strFilter)
{
    ASSERT(NULL == m_fp);

    char errbuf[PCAP_ERRBUF_SIZE];
    m_fp = pcap_open(strDevice.c_str(),
                     1514,      /* snaplen*/
                     PCAP_OPENFLAG_PROMISCUOUS, /*flags*/
                     20,        /* read timeout*/
                     NULL,      /* remote authentication */
                     errbuf);
    if (NULL == m_fp)
    {
        LogError(L"pcap_open(%hs) failed", strDevice.c_str());
        return false;
    }

    bpf_program fcode;
    bpf_u_int32 NetMask;

    // We should loop through the adapters returned by the pcap_findalldevs_ex()
    // in order to locate the correct one.
    //
    // Let's do things simpler: we suppose to be in a C class network ;-)
    NetMask = 0xffffff;

    //compile the filter
    if (pcap_compile(m_fp, &fcode, (char*)strFilter.c_str(), 1, NetMask) < 0)
    {
        LogError(L"pcap_compile(%hs) failed: %hs", strFilter.c_str(), GetErrorString());
        return false;
    }
    if (pcap_setfilter(m_fp, &fcode) < 0)
    {
        LogError(L"pcap_setfilter(%hs) failed: %hs", strFilter.c_str(), GetErrorString());
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
PcapTask_t::
EventHandler(
    DP::Event::Data_t& Data)
{
    HRESULT hr = DP::Handler_t::EventHandler(Data);
    if (S_FALSE != hr)
        return hr;
    using namespace DP::Event;
    switch (Data.Id)
    {
    case Id::Start:
        Start();
        break;
    case Id::Stop:
        Stop();
        break;
    default:
        return S_FALSE;
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool
PcapTask_t::
Start()
{
    // TODO: InitPcap
    LogInfo(L"PcapTask_t::Start()");
    ASSERT(INVALID_HANDLE_VALUE == m_hThread.get());
    HANDLE hThread = util::CreateThread(0, 0, ThreadFunc, (void *)this, 0, &m_dwThreadId);
    if (NULL == hThread)
        return false;
    m_hThread = hThread;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
PcapTask_t::
Stop()
{
    LogInfo(L"PcapTask_t::Stop(%d)", m_hThread.get());
    if (m_hThread.valid())
    {
        SignalObjectAndWait(m_hExitEvent.get(), m_hThread.get(), INFINITE, FALSE);
        m_hThread.Close();
    }
    // TODO: CleanupPcap
}

///////////////////////////////////////////////////////////////////////////////

DWORD
WINAPI
PcapTask_t::
ThreadFunc(
    void *pvParam)
{
	enum
	{
		Exit     = 0,
		cHandles,
	};
	PcapTask_t *pClass = static_cast<PcapTask_t*>(pvParam);
	HANDLE aHandles[cHandles];
	aHandles[Exit] = pClass->m_hExitEvent.get();

	for (;;)
    {
        pcap_pkthdr*  header;
        const u_char* pkt_data;

        // Read a packet
        int res;
        res = pcap_next_ex(pClass->m_fp, &header, &pkt_data);
        if (0 < res)
        {
            pClass->PostData(pkt_data);
        }
        else if (0 > res)
        {
            LogError(L"pcap_next_ex failed: %hs", pClass->GetErrorString());
        }

        // TODO: WaitForSingleObj?  get event handle from Pcap for multiple wait?
//		DWORD dw = WaitForMultipleObjects(cHandles, aHandles, FALSE, 20);
		DWORD dw = WaitForSingleObject(aHandles[Exit], 20);
		switch (dw)
		{
		case WAIT_OBJECT_0 + Exit:
            LogInfo(L"PcapTask_t::ThreadFunc()::Exit");
			util::ExitThread(0);
			break;
        case WAIT_TIMEOUT:
            break;
        default:
            LogError(L"PcapTask_t::ThreadFunc()::Wait returned %d", dw);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void
PcapTask_t::
PostData(
    const unsigned char* pkt_data)
{
pkt_data;
    // TODO: this could implement default propagation of a "packet"
}		

///////////////////////////////////////////////////////////////////////////////

const char *
PcapTask_t::
GetErrorString() const
{
    return pcap_geterr(m_fp);
}

///////////////////////////////////////////////////////////////////////////////
