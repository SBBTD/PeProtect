#pragma once
typedef struct _PACKINFO
{
	DWORD StartAddress;			//�洢��ʼ������ַ
	DWORD temp1[20];
	
	DWORD TargetOepRva;			//�����洢Ŀ������OEP��
	DWORD ImageBase;			//���ػ�ַ

	
	BOOL bIsTlsUserful;			//�Ƿ����TLS��
	
	DWORD  TlsCallbackFuncRva;// tls�ص�����ָ������
	DWORD TlsIndex;				//TLS�������
	

	DWORD ImportTableRva;		//IAT��rva
	DWORD RelocRva;				//�ض�λ��rva
 

	DWORD PackSectionNumber;	// ѹ����������
	DWORD packSectionRva;		// ѹ�����ε�rva
	DWORD packSectionSize;		//ѹ�����εĴ�С
	// ѹ��������ÿ�����ε�index�ʹ�С	
	// �±�1��ʾѹ������������ �±��[0]=ѹ��������� �±��[1]=ѹ���ļ���С 
	DWORD PackInfomation[50][2];


	
}PACKINFO , *PPACKINFO;

