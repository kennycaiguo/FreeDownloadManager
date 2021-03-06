/*
  Free Download Manager Copyright (c) 2003-2016 FreeDownloadManager.ORG
*/

#include "fsFtpConnection.h"
#include "fsInternetSession.h"
#include "common.h"
#include "system.h"
#include <stdio.h>

fsFtpConnection::fsFtpConnection()
{
	m_bPassiveMode = FALSE;
}

fsFtpConnection::~fsFtpConnection()
{

}

fsInternetResult fsFtpConnection::Connect(LPCTSTR pszServer, LPCTSTR pszUser, LPCTSTR pszPassword, INTERNET_PORT nPort)
{
	fsInternetResult ir;

	if (!m_pSession)
		return IR_NOTINITIALIZED;

	
	HINTERNET hSession = m_pSession->GetHandle ();

	if (!hSession)
		return IR_NOTINITIALIZED;

	if (m_hServer)
		InternetCloseHandle (m_hServer);

	m_hServer = InternetConnect (hSession, pszServer, nPort, pszUser, pszPassword, 
					INTERNET_SERVICE_FTP, m_bPassiveMode ? INTERNET_FLAG_PASSIVE : 0,
					NULL);

	if (m_hServer == NULL) 
	{
		ir = fsWinInetErrorToIR ();	
		DialogFtpResponse ();		
		if (ir == IR_EXTERROR)		
			ReceiveExtError ();		
		return ir;
	}

	DialogFtpResponse ();

	m_strServer = pszServer;
	m_uPort = nPort;

	return IR_SUCCESS;
}

void fsFtpConnection::UsePassiveMode(BOOL bUse)
{
	m_bPassiveMode = bUse;
}

void fsFtpConnection::ReceiveExtError()
{
	SAFE_DELETE_ARRAY (m_pszLastError);
	DWORD dwLen = 0;
	DWORD dwErr;

	InternetGetLastResponseInfo (&dwErr, NULL, &dwLen);

	if (::GetLastError () == ERROR_INSUFFICIENT_BUFFER)	
	{
		dwLen ++;	
		fsnew (m_pszLastError, TCHAR, dwLen);	
		InternetGetLastResponseInfo (&dwErr, m_pszLastError, &dwLen);	
	}
}

fsInternetResult fsFtpConnection::SetCurrentDirectory(LPCTSTR pszDir)
{
	if (pszDir == NULL || *pszDir == 0)
		return IR_ERROR;

	fsString strCmd;
	strCmd = _T("CWD ");
	strCmd += pszDir;
	Dialog (IFDD_TOSERVER, strCmd);	
									

	if (!FtpSetCurrentDirectory (m_hServer, pszDir))
	{
		fsInternetResult ir = fsWinInetErrorToIR ();
		if (ir == IR_EXTERROR)
			ReceiveExtError ();
		DialogFtpResponse ();
		return ir;
	}

	DialogFtpResponse ();	
	return IR_SUCCESS;
}

BOOL fsFtpConnection::IsPassiveMode()
{
	return m_bPassiveMode;
}
