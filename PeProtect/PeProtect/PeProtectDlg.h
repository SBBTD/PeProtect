
// PeProtectDlg.h : ͷ�ļ�
//

#pragma once
#include "afxeditbrowsectrl.h"

typedef bool(*Pack)(PCHAR pPath);

// CPeProtectDlg �Ի���
class CPeProtectDlg : public CDialogEx
{
// ����
public:
	CPeProtectDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PEPROTECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMFCEditBrowseCtrl m_MfcEditBrowse;
	CString m_Browse_FilePath;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedButtonPack();


private:
	HMODULE m_hModule;
	Pack	m_PackFunc;
public:
	// ѹ��
	BOOL m_bIsCompression;
	BOOL m_bIsEncryption;
};
