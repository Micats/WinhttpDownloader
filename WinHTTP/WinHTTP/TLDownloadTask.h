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

	CString         m_strUrl;           // ���ص�ַ
	CString         m_strAgent;         // �û�agent
	HWND            m_hWnd;             // �������ؽ�����Ϣ�Ĵ��ھ��
	LONG            *m_pTerminate;      // ָ���Ƿ���ֹ�ı�־λ��һ�����û����������������ȡ������ť�����Ĵ�ֵ
	int             m_nMaxTryCount;     // ������Դ������ض��������ԣ�Ĭ��20�Σ�
	int             m_nTimeoutSec;      // socket��ʱ���룬Ĭ��10�룩
	int             m_nPort;            // �˿ڣ�Ĭ��80��
	enum ProtocolType 
	{
		HTTP_PROTOCOL=80,
		HTTPS_PROTOCOL=443,
		USER_PROTOCOL=2
	};
	ProtocolType	m_proType;         //����http����https
public:

	CStringA        m_strAbsoluteUrlA;  //���������ص�ַ
	CStringA        m_strQueryA;		//����
	CStringA        m_strHostA;			//������ַ
	unsigned int    m_uReadBytes;		//�Ѿ����յ��ֽ�
	unsigned int    m_uTotalBytes;		//Ҫ���ص��������ֽ�
};

