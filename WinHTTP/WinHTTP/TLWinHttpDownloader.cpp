#include "stdafx.h"
#include"TLWinHttpDownloader.h"
#pragma comment(lib,"Winhttp.lib")
#include <math.h>
#include<time.h>
#include<iostream>
#include <conio.h>
#include"TLWebDef.h"
using namespace std;

#define BLOCK_SIZE 100
#define HTTP_STATUS_CODE 0

#define THE_CONNET_FAILED 1
#define THE_CONNET_SUCCESS 0

TLWinHttpDownloader::TLWinHttpDownloader()
{

}

TLWinHttpDownloader::~TLWinHttpDownloader()
{
	

}


DWORD TLWinHttpDownloader::DownloadToFile(TLDownloadTask &task, CString strOutputFile)
{
	//写进buffer
	TLByteBufferVector vec;
	DWORD dwRet=this->DownloadToBuffer(task, vec);
	if (THE_SUCCEED != dwRet)
	{
		return dwRet;
	}
	//写进文件
	HANDLE hFile = ::CreateFile(strOutputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	BYTE *pBuffer = vec.Ptr(0, task.m_uReadBytes);
	DWORD dwBytesWritten = 0;
	::WriteFile(hFile, pBuffer, task.m_uReadBytes, &dwBytesWritten, NULL);
	::CloseHandle(hFile);
	return (dwBytesWritten == task.m_uReadBytes) ? THE_SUCCEED : THE_WRITE_FILE;
}

int TLWinHttpDownloader::GetSleepSecCount(int nTryCount) const
{
	return (nTryCount + 1) * 1000;
}

int TLWinHttpDownloader::GetBufferSize(const TLDownloadTask & task) const
{
	if (0 == task.m_uTotalBytes)
	{
		return BLOCK_SIZE;
	}
	//return std::min<int>(BLOCK_SIZE, task.m_uTotalBytes - task.m_uReadBytes);
	return task.m_uTotalBytes - task.m_uReadBytes;
}

CStringA TLWinHttpDownloader::GenerateRequest(TLDownloadTask & task) const
{
	CStringA strRequest;

	CStringA strTemp;
	strRequest.Append("Accept-Language: zh-CN\r\n");

	strRequest.Append("Accept-Encoding: deflate\r\n");
	strRequest.Append("Connection: Keep-Alive\r\n");

	strTemp.Format("HOST: %s\r\n", task.m_strHostA.GetString());
	strRequest.Append(strTemp);

	strTemp.Format("Range: bytes=%d-\r\n\r\n", task.m_uReadBytes);
	strRequest.Append(strTemp);

	return strRequest;

}

DWORD TLWinHttpDownloader::ConnectServer(const TLDownloadTask & task)
{
	//连接服务器
	CString temp(task.GetAgentA());
	LPWSTR pointer = (LPWSTR)(LPCTSTR)temp;

	hSession = WinHttpOpen(pointer, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		wprintf(L"WinHttpOpen failed (0x%.8X)\n", GetLastError());
		return THE_CONNET_FAILED;
	}
	//printf("hSession success\n");
	CString temp1(task.m_strHostA);
	LPWSTR pointer1 = (LPWSTR)(LPCTSTR)temp1;
	hConnect = WinHttpConnect(hSession, pointer1, task.m_proType, 0); //协议类型
	if (!hConnect) {
		wprintf(L"WinHttpConnect failed (0x%.8X)\n", GetLastError());
		return THE_CONNET_FAILED;
	}
	//printf("hConnect success\n");

	//type接收类型
	LPCWSTR types[2];
	types[0] = L"text/html,application/xhtml+xml";
	types[1] = 0;  //\r\n
	//进行判断来确定是否使用ssl来支持HTTPS ，最后一个参数确定是否启用
	if (task.m_proType == task.HTTP_PROTOCOL)
	{
		CString temp(task.m_strAbsoluteUrlA.GetString());
		LPWSTR pointer = (LPWSTR)(LPCTSTR)temp;
		hRequest = WinHttpOpenRequest(hConnect, L"GET", pointer,
			NULL, WINHTTP_NO_REFERER, &types[0], 0);//WINHTTP_FLAG_SECURE
	}
	else if (task.m_proType == task.HTTPS_PROTOCOL)
	{
		CString temp(task.m_strAbsoluteUrlA.GetString());
		LPWSTR pointer = (LPWSTR)(LPCTSTR)temp;
		wprintf(L"https connect request");
		hRequest = WinHttpOpenRequest(hConnect, L"GET", pointer,
			NULL, WINHTTP_NO_REFERER, &types[0], WINHTTP_FLAG_SECURE);//WINHTTP_FLAG_SECURE
	}
	if (!hRequest)
	{
		wprintf(L"WinHttpOpenRequest failed (0x%.8X)\n", GetLastError());
		return THE_CONNET_FAILED;
	}
	return THE_CONNET_SUCCESS;
}

DWORD TLWinHttpDownloader::DoDownloadToBuffer(TLDownloadTask & task, TLByteBufferVector & bufVec)
{
	task.ParseUrl();
	cout << task.m_proType << " " << task.m_nPort << " " << task.m_strHostA << " " << task.m_strAgent << endl;
	DWORD dwRet=ConnectServer(task);
	if (THE_CONNET_SUCCESS!=dwRet)
	{
		closeWinHTTP();
		return dwRet;
	}
	dwRet=DoDownloadToBufferInner(task, bufVec);
	closeWinHTTP();
	return dwRet;
}

DWORD TLWinHttpDownloader::DoDownloadToBufferInner(TLDownloadTask & task, TLByteBufferVector & bufVec)
{
	//生成请求头并request
	CStringA strRequest = this->GenerateRequest(task);
	CString temp(GenerateRequest(task));
	LPWSTR pointer = (LPWSTR)(LPCTSTR)temp;
	BOOL bResults;
	if (!(bResults = WinHttpSendRequest(hRequest, pointer, -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)))
	{
		wprintf(L"WinHttpSendRequest failed (0x%.8X)\n", GetLastError());
	}

	//接受reponse,
	if (!(bResults = WinHttpReceiveResponse(hRequest, 0)))
	{
		wprintf(L"WinHttpReceiveResponse failed (0x%.8X)\n", GetLastError());
	}
	LPVOID lpHeaderBuffer = NULL;
	DWORD dwSize = 0;

	//获取完整的header,接受处写NULL，然后可以在错误中返回长度
	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX, NULL,
		&dwSize, WINHTTP_NO_HEADER_INDEX);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		lpHeaderBuffer = new WCHAR[dwSize / sizeof(WCHAR)];
		//此时申请相应buff
		bResults = WinHttpQueryHeaders(hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX,
			lpHeaderBuffer, &dwSize,
			WINHTTP_NO_HEADER_INDEX);
	}
	printf("Header contents: \n%S", lpHeaderBuffer);
	delete lpHeaderBuffer;

	//获取请求的字节
	wchar_t szContentLength[32];
	DWORD dwContentLength = 0;
	BOOL haveContentLength = WinHttpQueryHeaders(
		hRequest,
		WINHTTP_QUERY_CONTENT_LENGTH,
		NULL,
		&szContentLength,
		&dwSize,
		WINHTTP_NO_HEADER_INDEX
		);
	if (haveContentLength)
	{
		dwContentLength = _wtoi(szContentLength);
	}
	printf("Toltal data:%d\n", dwContentLength);

	//获取状态码
	wchar_t codeStatus[32];
	int statusVal = 0;
	bool ret = WinHttpQueryHeaders(
		hRequest,
		WINHTTP_QUERY_STATUS_CODE,
		NULL,
		&codeStatus,
		&dwSize,
		WINHTTP_NO_HEADER_INDEX
		);
	if (!ret)
	{
		printf("status code error:%d\n", GetLastError());
	}
	statusVal = _wtoi(codeStatus);
	printf("status code:%d\n", statusVal);

	//处理状态码
	if (statusVal == HTTP_STATUS_PARTIAL_CONTENT)	    // 206: 断点续传
	{
		task.m_uTotalBytes = task.m_uReadBytes + dwContentLength;
	}
	else if (statusVal == HTTP_STATUS_OK)               // 200: 重新下载（服务器不支持断点续传）
	{
		task.m_uTotalBytes =task.m_uReadBytes+dwContentLength;
		task.m_uReadBytes = 0;
		bufVec.Reset();
	}
	else if (statusVal == HTTP_STATUS_REDIRECT)         // 302: 重定向
	{
		//暂未测试，未找到例子，查询到80%概率是自动重定向找到了网址
		return THE_REDIRECT;
	}
	else
	{
		task.m_uTotalBytes = task.m_uReadBytes + dwContentLength;
		//return THE_INVALID_STATUS_CODE;
	}
	printf("m_uReadBytes:%d\n", task.m_uReadBytes);
	printf("task.m_uTotalBytes:%d\n", task.m_uTotalBytes);

	if (task.m_uReadBytes < task.m_uTotalBytes)
	{

		int nBufferSize = this->GetBufferSize(task);
		cout << "nBufferSize:" << nBufferSize << endl;
		BYTE* pBuffer = bufVec.Ptr(task.m_uReadBytes, nBufferSize);
		//接受返回的数据
		DWORD dwDownloaded = 0;//实际收取的字符数
		if (bResults)
		{
			do
			{
				//分dwSize来接受数据（以字节为单位）
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
					cout << "Error：WinHttpQueryDataAvailable failed：" << GetLastError() << endl;
					break;
				}
				if (!dwSize)
				{
					break;  //数据大小为0 
				}

				//通过WinHttpReadData读取服务器的返回数据
				if (!WinHttpReadData(hRequest, (char *)pBuffer, dwSize, &dwDownloaded)) {
					cout << "Error：WinHttpQueryDataAvailable failed：" << GetLastError() << endl;
				}
				pBuffer += dwDownloaded;
				task.m_uReadBytes += dwDownloaded;

				//显示百分比
				HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE);
				COORD pos = { 0, 25 };
				SetConsoleCursorPosition(hd, pos);
				printf("Received data: %d%%\n", task.Percentage());
				if (!dwDownloaded)
					break;

				//当按下ESC时循环，ESC键的键值时27，测试断点续传
				int ch;
				if (_kbhit()) {
					ch = _getch();
					cout << ch;
					if (ch == 27)
					{
						printf("m_uReadBytes:%d\n", task.m_uReadBytes);
						printf("m_uTotalBytes:%d\n", task.m_uTotalBytes);
						break;
					}
				}

			} while (dwSize > 0);
			//如果读的不是二进制而是网页字符串之类的，可以转换编码，
			//DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, pszOutBuffer, -1, NULL, 0);    //返回原始ASCII码的字符数目       
			//pwText = new wchar_t[dwNum];                                                //根据ASCII码的字符数分配UTF8的空间
			//MultiByteToWideChar(CP_UTF8, 0, pszOutBuffer, -1, pwText, dwNum);           //将ASCII码转换成UTF8
			//printf("Received contents: \n%S", pwText);
		}
	}

	return 0;
}

void TLWinHttpDownloader::closeWinHTTP()
{
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	printf("Close Link\n");
}

DWORD TLWinHttpDownloader::DownloadToBuffer(TLDownloadTask &task, TLByteBufferVector &bufVec)
{
	int nTryCount = 0;
	DWORD dwRet = DoDownloadToBuffer(task, bufVec);
	if (THE_REDIRECT != dwRet)
	{
		nTryCount++;
	}
	while (dwRet != THE_SUCCEED    &&              
		   dwRet != THE_USER_CANCELED2  &&
		   nTryCount < task.m_nMaxTryCount)                //尝试重试
	{
		int nTime = this->GetSleepSecCount(nTryCount);
		if (::InterlockedCompareExchange(task.m_pTerminate, 1, 1))
		{
			return THE_USER_CANCELED2;
		}
		dwRet = this->DoDownloadToBuffer(task, bufVec);
		if (THE_REDIRECT != dwRet)
		{
			nTryCount++;
		}
	}

	return dwRet;
}
