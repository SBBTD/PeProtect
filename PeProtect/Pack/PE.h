#pragma once
#include <Windows.h>
#include <string.h>
#include "stdlib.h"
#include "..\\Stub\\Stub.h"
class CPe
{
public:
	CPe();
	~CPe();
public:
	//��ȡOEP
	DWORD GetOepRva();
	//��ȡ�ļ����ڴ�
	bool ReadTargetFile(char* pPath, PPACKINFO&  pPackInfo);
	//���������
	DWORD AddSection(
		PCHAR szName ,        //�����ε�����
		PCHAR pSectionBuf ,   //�����ε�����
		DWORD dwSectionSize , //�����εĴ�С
		DWORD dwAttribute    //�����ε�����
	);

	//�޸����ص�dll�ض�λ
	void FixDllRloc(PCHAR pBuf,PCHAR pOri);
	//����
	void Encryption();
	//ȥ�������ַ
	void CancleRandomBase();
	//��ȡ������ַ
	DWORD GetImportTableRva();
	//��ȡ�ض�λ���ַ
	DWORD GetRelocRva();
	//���������0
	void ChangeImportTable();
	//��ȡ���ػ�ַ
	DWORD GetImageBase();
	//������������Ϊ��д
	void SetMemWritable();
	//��Ŀ������ض�λָ��ָ��Ϊdll�ض�λ��
	void ChangeReloc(PCHAR pBuf);
	//��ȡ�����ε�ַ
	DWORD GetNewSectionRva();
	//��ȡ������ε�ַ
	DWORD GetLastSectionRva();
	//��ȡ�ļ�ƫ��
	DWORD RvaToOffset(DWORD Rva);

	//ѹ��
	void EnCompression(PPACKINFO& pPackInfo);

	//����ѹ����
	PCHAR Compress(PVOID pSource, IN long InLength, OUT long &OutLength);

	//��ȡ�����Σ���һ�����ĵ�ַ
	DWORD GetFirstNewSectionRva();
	//��ȡ�µ�OEP
	void SetNewOep(DWORD dwNewOep);
	//�����ļ�
	void SaveNewFile(char* pPath);

	//��������С
	DWORD  CalcAlignment(DWORD dwSize , DWORD dwAlignment);

	//�޸�Tls��
	BOOL ModifyTlsTable(PPACKINFO & pPackInfo);

	void SetTls(DWORD NewSectionRva, PCHAR pStubBuf, PPACKINFO pPackInfo);

	BOOL DealwithTLS(PPACKINFO & pPackInfo);

private:
	// Ŀ�������������
	DWORD m_SectionNumber;
	// �������������
	DWORD m_codeIndex;


	DWORD m_pResRva;				//��Դ���ַ
	DWORD m_pResSectionRva;			//��Դ�ε�ַ
	DWORD m_ResSectionIndex;		//��Դ���������е����
	DWORD m_ResPointerToRawData ;	// ��Դ�����ļ��е�ƫ��
	DWORD m_ResSizeOfRawData ;		//	��Դ�δ�С


	DWORD m_pTlsDataRva;			// �洢tls���ݵ�����,Ҳ����.tls����
	DWORD m_pTlsSectionRva;			//Tls�ε�ַ
	DWORD m_TlsSectionIndex;
	DWORD m_TlsPointerToRawData;
	DWORD m_TlsSizeOfRawData;


private:
	//TLS���е���Ϣ
	DWORD m_StartOfDataAddress;		//TLS���ο�ʼ��ַ
	DWORD m_EndOfDataAddress;		//TLS���ν�����ַ
	DWORD m_CallBackFuncAddress;	//TLS�ص����������ַ
	
private:
	//ԭĿ������ڴ�
	PCHAR m_pBuf;			//�ļ�����
	DWORD m_FileSize;
private:
	//�¿��ٿռ临�Ƶ�Ŀ�����
	PCHAR m_pNewBuf;		//�ļ�����
	DWORD m_dwNewFileSize;

	PIMAGE_DOS_HEADER m_pDos;
	PIMAGE_NT_HEADERS m_pNt;
	PIMAGE_SECTION_HEADER m_pSection;

};

