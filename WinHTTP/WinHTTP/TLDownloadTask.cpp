#include "stdafx.h"
#include "TLDownloadTask.h"
#include <time.h>

#define INTERNET_DEFAULT_HTTP_PORT 80
#define INTERNET_DEFAULT_HTTPS_PORT 443

const int BLOCK_SIZE = 1024 * 64;
const int DEFAULT_MAX_TRY = 1;
const int DEFAULT_TIMEOUT = 5;

TLDownloadTask::TLDownloadTask():m_nPort(INTERNET_DEFAULT_HTTP_PORT),
m_nMaxTryCount(DEFAULT_MAX_TRY),
m_uReadBytes(0),
m_uTotalBytes(0),
m_nTimeoutSec(DEFAULT_TIMEOUT),
m_hWnd(NULL),
m_pTerminate(NULL),
m_proType(HTTP_PROTOCOL)
{
}

CStringA TLDownloadTask::GetUrlA() const
{
	return CStringA(m_strUrl);
}

CStringA TLDownloadTask::GetAgentA() const
{
	return CStringA(m_strAgent);
}

void TLDownloadTask::ParseUrl()
{
	m_strAbsoluteUrlA = m_strHostA = m_strQueryA = "";

	CStringA strUrlA = this->GetUrlA();
	const char *pUrl = strUrlA;
	const char *p = pUrl;
	const char *szHttpHead = "http://";
	const char *szHttpsHead = "https://";
	if (_strnicmp(pUrl, szHttpHead, strlen(szHttpHead)) == 0)//URL地址开头是http
	{
		p = pUrl + strlen(szHttpHead);
		m_nPort = INTERNET_DEFAULT_HTTP_PORT;
		m_proType = HTTP_PROTOCOL;
	}
	if (_strnicmp(pUrl, szHttpsHead, strlen(szHttpsHead)) == 0)//URL地址开头是https
	{
		p = pUrl + strlen(szHttpsHead);
		m_nPort = INTERNET_DEFAULT_HTTPS_PORT;
		m_proType = HTTPS_PROTOCOL;
	}
	int nHostLen = 0;
	const char *q = strchr(p, '/');
	if (q != NULL)
	{
		nHostLen = q - p;
		int nPathLen = 0;
		const char *r = strchr(q, '?');
		if (r != NULL)
		{
			// 解析query
			r++;
			m_strQueryA = r;
			nPathLen = r - q - 1;
		}
		else
		{
			nPathLen = strlen(q);
		}

		// 解析abs_path
		m_strAbsoluteUrlA.Append(q, nPathLen);
	}
	else
	{
		nHostLen = strlen(p);
	}

	// 解析host
	m_strHostA.Append(p, nHostLen);

	// 解析port
	const char *r = strchr(m_strHostA, ':');
	if (r == 0)
	{
		//m_nPort = INTERNET_DEFAULT_HTTP_PORT;
	}
	else
	{
		m_nPort = atoi(r + 1);
		m_strHostA = m_strHostA.Left(m_strHostA.ReverseFind(_T(':')));
	}
}

int TLDownloadTask::Percentage() const
{
	return (m_uTotalBytes == 0)
		? 0
		: (int)((unsigned long long)m_uReadBytes * 100 / (unsigned long long) m_uTotalBytes);
}

DWORD TLDownloadTask::RemainTimeSec(DWORD dwTickElapsed, unsigned int uBytesTransferred) const
{
	unsigned long long uTickElapsed = (unsigned long long)dwTickElapsed;
	unsigned long long uBytes = (unsigned long long)uBytesTransferred;
	unsigned long long uRemain = (unsigned long long)(m_uTotalBytes - m_uReadBytes);
	//Log(_T("elapsed=%d, get=%d, remain=%d\n"), dwTickElapsed, uBytesTransferred, m_uTotalBytes - m_uReadBytes);
	return (DWORD)(uTickElapsed * uRemain / (uBytes * CLOCKS_PER_SEC));
}

unsigned int TLDownloadTask::ReadBytes() const
{
	return m_uReadBytes;
}
