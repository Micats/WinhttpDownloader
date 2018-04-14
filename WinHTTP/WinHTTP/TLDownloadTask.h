#pragma once
#include <atlstr.h>
#include"TLDownloadTask.h"

class TLWinHttpDownloader;

class TLDownloadTask
{
	friend class TLWinHttpDownloader;
public:
	TLDownloadTask();
	
	CStringA GetUrlA() const;
	CStringA GetAgentA() const;
	void ParseUrl();

	int Percentage() const;
	DWORD RemainTimeSec(DWORD dwTickElapsed, unsigned int uBytesTransferred) const;
	unsigned int ReadBytes() const;

	CString         m_strUrl;           // 下载地址
	CString         m_strAgent;         // 用户agent
	HWND            m_hWnd;             // 接收下载进度消息的窗口句柄
	LONG            *m_pTerminate;      // 指向是否中止的标志位，一般由用户界面操作（如点击“取消”按钮）更改此值
	int             m_nMaxTryCount;     // 最多重试次数（重定向不算重试，默认20次）
	int             m_nTimeoutSec;      // socket超时（秒，默认10秒）
	int             m_nPort;            // 端口（默认80）
	enum ProtocolType 
	{
		HTTP_PROTOCOL=80,
		HTTPS_PROTOCOL=443,
		USER_PROTOCOL=2
	};
	ProtocolType	m_proType;         //采用http还是https
public:

	CStringA        m_strAbsoluteUrlA;  //完整的下载地址
	CStringA        m_strQueryA;		//参数
	CStringA        m_strHostA;			//主机地址
	unsigned int    m_uReadBytes;		//已经接收的字节
	unsigned int    m_uTotalBytes;		//要下载的任务总字节
};

