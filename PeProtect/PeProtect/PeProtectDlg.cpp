
// PeProtectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PeProtect.h"
#include "PeProtectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPeProtectDlg �Ի���



CPeProtectDlg::CPeProtectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PEPROTECT_DIALOG, pParent)
	, m_Browse_FilePath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPeProtectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_MfcEditBrowse);
	DDX_Text(pDX, IDC_MFCEDITBROWSE1, m_Browse_FilePath);
}

BEGIN_MESSAGE_MAP(CPeProtectDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CPeProtectDlg::OnBnClickedButtonPack)
END_MESSAGE_MAP()


// CPeProtectDlg ��Ϣ�������

BOOL CPeProtectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//����ChangeWindowMessageFilter����������WM_DROPFILES��Ϣ��WM_COPYGLOBALDATA��Ϣ���Խ��Win7ϵͳ���ļ��Ϸ�ʧЧ������
	DragAcceptFiles(TRUE);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);       // 0x0049 == WM_COPYGLOBALDATA

	if (!(m_hModule = LoadLibraryA("..\\Debug\\Pack.dll")))
	{
		MessageBox(L"����Pack.dllʧ��");
		TerminateProcess(NULL, 0);
	}
	else
	{
		m_PackFunc = (Pack)GetProcAddress(m_hModule, "Pack");
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPeProtectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPeProtectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPeProtectDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnDropFiles(hDropInfo);

	//�϶������ļ���ʱ��
	TCHAR szPath[MAX_PATH] = {};
	//��ȡ��ק�ļ���·��
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	m_Browse_FilePath = szPath;
	//��·����ӵ��ļ�����
	m_MfcEditBrowse.SetWindowText(m_Browse_FilePath);
	//UpdateData(FALSE);
	//�ϷŽ������ͷ��ڴ�
	DragFinish(hDropInfo);
	UpdateData(TRUE);
	//MessageBox(m_Browse_FilePath, L"Tip", 0);
}


void CPeProtectDlg::OnBnClickedButtonPack()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	m_bIsCompression = 0;				//�Ƿ�ѹ��
	m_bIsEncryption = 0;				//�Ƿ����
	
	char *szPath = new char[255];
	WideCharToMultiByte(CP_ACP, 0, m_Browse_FilePath.GetBuffer(), -1, szPath, 255, NULL, FALSE);

	UpdateData(TRUE);

	if (m_PackFunc(szPath))
	{
		MessageBox(L"���...\n", L"Tip", NULL);
	}
	delete[] szPath;
}
