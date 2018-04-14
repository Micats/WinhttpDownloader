// WinHTTP.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "TLBufferVector.h"
#include "TLWinHttpDownloader.h"
#include "TLDownloadTask.h"

using namespace std;




int main()
{
	TLDownloadTask task;
	//task.m_strUrl = "http://f1.market.xiaomi.com/download/AppStore/0831c047a8a8e4cff181d1466c39f5d4145402b80/com.julanling.app.apk";
	//task.m_strUrl = "https://wwwali.csc108.com/download/XtAmpTradeClient_x64_zxjt_3.0.1.10660.exe";
	task.m_strUrl = "https://wwwali.csc108.com/download/update64.cfg";
	task.m_strAgent = "userClient";
	task.m_uReadBytes = 0;
	;

	TLByteBufferVector vecBuf;

	TLWinHttpDownloader down;
	
	//down.DownloadToBuffer(task, vecBuf);
	down.DownloadToFile(task, "update64.cfg");


	system("pause");
	return 0;
}

