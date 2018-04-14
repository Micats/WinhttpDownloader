#pragma once
#include "TLDownloadTask.h"
#include<winhttp.h>
#include "TLBufferVector.h"

class TLWinHttpDownloader
{
public:
	TLWinHttpDownloader();
	virtual ~TLWinHttpDownloader();

	
	//�ⲿ���ýӿ�
	DWORD DownloadToBuffer(TLDownloadTask &task, TLByteBufferVector &bufVec);
	// ���ص�һ���ļ�
	DWORD DownloadToFile(TLDownloadTask &task, CString strOutputFile);


protected:
	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;
	HINTERNET hRequest = 0;
	char *buff;
	//TLDownloadTask m_task;
protected:
	int GetSleepSecCount(int nTryCount) const;
	int GetBufferSize(const TLDownloadTask &task) const;

	CStringA GenerateRequest(TLDownloadTask &task) const;

	DWORD ConnectServer(const TLDownloadTask &task);
	DWORD DoDownloadToBuffer(TLDownloadTask &task, TLByteBufferVector &bufVec);
	DWORD DoDownloadToBufferInner(TLDownloadTask &task, TLByteBufferVector &bufVec);
	void closeWinHTTP();
};

