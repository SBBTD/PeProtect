
// PeProtect.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPeProtectApp: 
// �йش����ʵ�֣������ PeProtect.cpp
//

class CPeProtectApp : public CWinApp
{
public:
	CPeProtectApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPeProtectApp theApp;