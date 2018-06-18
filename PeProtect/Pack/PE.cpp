#include "stdafx.h"
#include "PE.h"


CPe::CPe()
{
}


CPe::~CPe()
{
}



//��ȡĿ��������ڵ�Rva
DWORD CPe::GetOepRva()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pBuf);
	return pNt->OptionalHeader.AddressOfEntryPoint;
}

//��ȡҪ�����ļ����ڴ�
#pragma comment(lib, "User32.lib")	//messageboxa
bool CPe::ReadTargetFile(char* pPath, PPACKINFO& pPackInfo)
{
	DWORD dwRealSize = 0;
	//1 ���ļ�
	HANDLE hFile = CreateFileA(
		pPath , 0x0001 , FILE_SHARE_READ ,
		NULL ,
		OPEN_EXISTING ,
		FILE_ATTRIBUTE_NORMAL , NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "��ȡ�ļ�ʧ��...", "Tip", NULL);
		return FALSE;
	}


	//2 ��ȡ�ļ���С
	m_FileSize = GetFileSize(hFile , NULL);
	m_dwNewFileSize = m_FileSize;
	//3 �����¿ռ��������Ŀ���ļ�
	m_pBuf = new CHAR[m_FileSize];
	m_pNewBuf = m_pBuf;
	memset(m_pBuf , 0 , m_FileSize);

	//4 ���ļ����ݶ�ȡ��������Ŀռ���
	ReadFile(hFile , m_pBuf , m_FileSize , &dwRealSize , NULL);

	//��ȡĿ���ļ���PE��Ϣ
	m_pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	m_pNt = (PIMAGE_NT_HEADERS)(m_pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(m_pNt);
	// ����ԭʼ������
	m_SectionNumber = m_pNt->FileHeader.NumberOfSections;

	// ��ȡOEP
	DWORD dwOEP = m_pNt->OptionalHeader.AddressOfEntryPoint;

	// �����Դ�ε���Ϣ
	m_pResRva = m_pNt->OptionalHeader.DataDirectory[2].VirtualAddress;

	//��ʼ����Դ����Ϣ
	m_pResSectionRva = 0;		//��Դ�ε�ַ
	m_ResSectionIndex = -1;		//��Դ�������������е����
	m_ResPointerToRawData = 0;	//��Դ�����ļ��е�ƫ��
	m_ResSizeOfRawData = 0;		//��Դ�����ļ��еĴ�С

	//��ȡtls������Ϣ����ʼ��
	m_pTlsSectionRva = 0;		//Tls�ε�ַ
	m_TlsSectionIndex = -1;		//Tls�� �����������е����
	m_TlsPointerToRawData = 0;	//Tls�����ļ��е�ƫ��
	m_TlsSizeOfRawData = 0;		//Tls�����ļ��д�С
	if (m_pNt->OptionalHeader.DataDirectory[9].VirtualAddress)	//���Ŀ���ļ��д���Tls��
	{
		//��ȡTls��ָ��
		PIMAGE_TLS_DIRECTORY32 g_lpTlsDir = (PIMAGE_TLS_DIRECTORY32)
			(RvaToOffset(m_pNt->OptionalHeader.DataDirectory[9].VirtualAddress) + m_pNewBuf);
		//��ȡTls������ʼRVA
		m_pTlsDataRva = g_lpTlsDir->StartAddressOfRawData + m_pNt->OptionalHeader.ImageBase;
	}

	//��ȡ�����.text����Դ�����ڵ�������ţ���0��ʼ��
	for(int i = 0; i < m_pNt->FileHeader.NumberOfSections; i++)
	{
		// ���oep���������,���ж���������Ǵ����
		if(dwOEP >= pSection->VirtualAddress &&
		   dwOEP <= pSection->VirtualAddress + pSection->Misc.VirtualSize)
		{
			// ��ȡ����������������[ͨ��oep�ж�]
			m_codeIndex = i;
		}

		// ��ȡrsrc�ε���Ϣ
		if(m_pResRva >= pSection->VirtualAddress &&
		   m_pResRva <= pSection->VirtualAddress + pSection->Misc.VirtualSize)
		{
			m_pResSectionRva = pSection->VirtualAddress;
			m_ResPointerToRawData = pSection->PointerToRawData;
			m_ResSizeOfRawData = pSection->SizeOfRawData;
			m_ResSectionIndex = i;
		}
		
		//��ȡTls��Ϣ
		if (m_pNt->OptionalHeader.DataDirectory[9].VirtualAddress)	//�������TLs
		{
			if (m_pTlsDataRva >= pSection->VirtualAddress&&
				m_pTlsDataRva <= pSection->VirtualAddress + pSection->Misc.VirtualSize)
			{
				m_pTlsSectionRva = pSection->VirtualAddress;
				m_TlsSectionIndex = i;
				m_TlsPointerToRawData = pSection->PointerToRawData;
				m_TlsSizeOfRawData = pSection->SizeOfRawData;
			}
		}
		pSection = pSection + 1;
	}

	//5 �ر��ļ�
	CloseHandle(hFile);
	return TRUE;
}


// ���ڽ�PE�ļ���rvaתΪ�ļ�ƫ��
DWORD CPe::RvaToOffset(DWORD Rva)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	for (int i= 0; i<pNt->FileHeader.NumberOfSections; i++)
	{
		if(Rva >= pSection->VirtualAddress&&
		   Rva <= pSection->VirtualAddress+pSection->Misc.VirtualSize)
		{
			// ����ļ���ַΪ0,���޷����ļ����ҵ���Ӧ������
			if (pSection->PointerToRawData == 0)
			{
				return -1;
			}
			return Rva - pSection->VirtualAddress + pSection->PointerToRawData;
		}
		pSection = pSection + 1;
	}
}




//�������
DWORD CPe::AddSection(
	PCHAR szName ,        //�����ε�����
	PCHAR pSectionBuf ,   //�����ε�����
	DWORD dwSectionSize , //�����εĴ�С
	DWORD dwAttribute    //�����ε�����
)
{
	//1 ���ݸղŶ�ȡ��exe�ļ������ݣ��õ���������κ��µ�exe�ļ��Ĵ�С
	m_dwNewFileSize = m_FileSize + CalcAlignment(dwSectionSize , 0x200);
	//2 ����ռ�
	m_pNewBuf = new CHAR[ m_dwNewFileSize ];
	memset(m_pNewBuf , 0 , m_dwNewFileSize);
	//3 ��ԭ����PE���ݿ�����������Ŀռ���
	memcpy(m_pNewBuf , m_pBuf , m_FileSize);
	//4 �������ο�����PE�ļ��ĺ���
	memcpy(m_pNewBuf + m_FileSize , pSectionBuf , dwSectionSize);
	//5 �޸����α�
	m_pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	m_pNt = (PIMAGE_NT_HEADERS)(m_pDos->e_lfanew + m_pNewBuf);
	m_pSection = IMAGE_FIRST_SECTION(m_pNt);
	//�õ����α�����һ��
	PIMAGE_SECTION_HEADER pLastSection =
		m_pSection + m_pNt->FileHeader.NumberOfSections - 1;
	//�õ����α�����һ��ĺ���
	PIMAGE_SECTION_HEADER pNewSection = pLastSection + 1;
	pNewSection->Characteristics = dwAttribute;    //����
	strcpy_s((char *)pNewSection->Name , 32 , szName);//������--->�˴�������,����㲻����Ϊ֮����ռ�,������ӽ���ͷʱ���ܻ�Խ��.

	// �����ڴ�ƫ�ƺ��ڴ��С
	pNewSection->Misc.VirtualSize = dwSectionSize; //�ڴ��еĴ�С������Ҫ���룩
	pNewSection->VirtualAddress = pLastSection->VirtualAddress +
		CalcAlignment(pLastSection->Misc.VirtualSize , 0x1000);
	pNewSection->SizeOfRawData = CalcAlignment(dwSectionSize , 0x200);

	// �����ļ�ƫ�ƺ��ļ���С
	while (TRUE)
	{
		if (pLastSection->PointerToRawData)
		{
			// �ҵ�ǰһ����0������
			pNewSection->PointerToRawData = pLastSection->PointerToRawData +
				pLastSection->SizeOfRawData;
			break;
		}
		pLastSection = pLastSection - 1;
	}

	//6 �޸����������;����С
	m_pNt->FileHeader.NumberOfSections++;
	m_pNt->OptionalHeader.SizeOfImage = pNewSection->VirtualAddress + dwSectionSize;



	// ����һ�ݵ�ǰ�Ĵ�С
	m_FileSize = m_dwNewFileSize;

	// �ͷ�֮ǰ���ڴ�,������Ŀ���ļ�ռ���ڴ��С
	delete[] m_pBuf;
	m_pBuf = m_pNewBuf;

	// ������������ε�rva
	return pNewSection->VirtualAddress;
}

//��ȡ��һ�������ε�rva
DWORD CPe::GetFirstNewSectionRva()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	PIMAGE_SECTION_HEADER pLastSection = pSection + m_SectionNumber - 1;

	return pLastSection->VirtualAddress +
		CalcAlignment(pLastSection->Misc.VirtualSize , 0x1000);
}

//�����µĳ�����ڵ�
void CPe::SetNewOep(DWORD dwNewOep)
{
	m_pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	m_pNt = (PIMAGE_NT_HEADERS)(m_pDos->e_lfanew + m_pNewBuf);
	m_pNt->OptionalHeader.AddressOfEntryPoint = dwNewOep;
}

//�����ļ�
void CPe::SaveNewFile(char* pPath)
{
	//1 ���ļ�
	DWORD dwRealSize = 0;
	HANDLE hFile = CreateFileA(
		pPath , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ ,
		NULL ,
		OPEN_ALWAYS ,
		FILE_ATTRIBUTE_NORMAL , NULL
	);
	//2 ���ڴ��е�����д�뵽�ļ���
	WriteFile(hFile ,
			  m_pNewBuf , m_dwNewFileSize , &dwRealSize , NULL);
	//3 �ر��ļ������
	CloseHandle(hFile);
}


//��ȡ�����Ĵ�С
DWORD  CPe::CalcAlignment(DWORD dwSize , DWORD dwAlignment)
{
	if(dwSize%dwAlignment == 0)
	{
		return dwSize;
	}
	else
	{
		return (dwSize / dwAlignment + 1)*dwAlignment;
	}
}
	
//************************************
// ������:	FixRloc
// ����:	���������εĵ�ַ�޸�dll���ض�λ[dll�Ǽ��ص��ڴ��,�������Ĭ�ϼ��ػ�ַ,����ӵĽ�����rva�Լ���ԭ������ʼ�Ĳ�ֵ����������.text���ض�λ]
// ����ֵ:	void
// ����:	PCHAR pStubBuf, ����stubdll���ڴ��׵�ַ
// ����:    PCHAR pStub(����ȷ��dll����ʱ�ض�λ)
//************************************
void CPe::FixDllRloc(PCHAR pStubBuf, PCHAR pStub)
{
	// �����ض�λ��Ϣ�ṹ��
	typedef struct _TYPE
	{
		unsigned short offset : 12;
		unsigned short type : 4;
	}TYPE , *PTYPE;
	
	//��λ����һ���ض�λ��
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pStubBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + pStubBuf);
	PIMAGE_DATA_DIRECTORY pRelocDir = (pNt->OptionalHeader.DataDirectory + 5);
	PIMAGE_BASE_RELOCATION pReloc =
		(PIMAGE_BASE_RELOCATION)(pRelocDir->VirtualAddress + pStubBuf);

	// ��ʼ�޸��ض�λ
	while(pReloc->SizeOfBlock != 0)
	{
		// �ض�λ�ʼ����
		DWORD BeginLoc = (DWORD)(pReloc->VirtualAddress + pStubBuf);
		// �ض�λ��ĸ���
		DWORD dwCount = (pReloc->SizeOfBlock - 8) / 2;
		// �ض�λ����
		PTYPE pType = (PTYPE)(pReloc + 1);
		// �޸�ÿһ���ض�λ��
		for(size_t i = 0; i < dwCount; i++)
		{
			// ���������3
			if(pType->type == 3)
			{
				// ��ȡ�ض�λ��ַ
				PDWORD pReloction = (PDWORD)(pReloc->VirtualAddress + pType->offset + pStubBuf);
				// ��ȡ���ض�λ��ַ���ض�λ�������ͷ��ƫ��: �ض�λ��ַrva - ģ��rva - ����rva
				DWORD Chazhi = *pReloction - (DWORD)pStub - 0x1000;		
				// ��ƫ�Ƽ����½�����rva��ø��ض�λ���rva,�ڼ��ϵ�ǰĬ�ϼ��ػ�ַ�����޸��ض�λ
				*pReloction = Chazhi + GetNewSectionRva() + GetImageBase();
			}
			//��λ����һ���ض�λ��
			pType++;
		}
		// ��λ����һ���ض�λ��
		pReloc = (PIMAGE_BASE_RELOCATION)((PCHAR)pReloc + pReloc->SizeOfBlock);
	}
}


//�Դ���ν��м���
void CPe::Encryption()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	// ���ݱ���Ĵ������ţ���λ�������,��������μ���
	pSection = pSection + m_codeIndex;
	PCHAR pStart = pSection->PointerToRawData + m_pNewBuf;
	for(int i = 0; i < (pSection->Misc.VirtualSize); i++)
	{
		pStart[i] ^= 0x15;
	}

}

//ȥ���ض�λ
void CPe::CancleRandomBase()
{
	m_pNt->OptionalHeader.DllCharacteristics &=
		~IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
}

//��ȡ������rva
DWORD CPe::GetImportTableRva()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pBuf);
	return pNt->OptionalHeader.DataDirectory[ 1 ].VirtualAddress;
}

//��ȡ�ض�λ���rva
DWORD CPe::GetRelocRva()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pBuf);
	return pNt->OptionalHeader.DataDirectory[5].VirtualAddress;
}

//�Ե������и���
void CPe::ChangeImportTable()
{
	// 3.��Ŀ¼��ĵ��������0
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	pNt->OptionalHeader.DataDirectory[ 1 ].VirtualAddress = 0;
	pNt->OptionalHeader.DataDirectory[ 1 ].Size = 0;

	pNt->OptionalHeader.DataDirectory[ 12 ].VirtualAddress = 0;
	pNt->OptionalHeader.DataDirectory[ 12 ].Size = 0;

}

//��ȡĿ�������ػ�ַ
DWORD CPe::GetImageBase()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pBuf);
	return pNt->OptionalHeader.ImageBase;
}

//����ÿ������Ϊ��д״̬
void CPe::SetMemWritable()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	DWORD SectionNumber = pNt->FileHeader.NumberOfSections;

	for (int i = 0; i < SectionNumber; i++)
	{
		pSection[ i ].Characteristics |= 0x80000000;
	}
}

//�ض�λ˼·��FixDllRloc�޸����ص��ڴ��stub.dll�ض�λ��ChangeReloc��ϵͳPE�ض�λָ��stub.dll�ض�λ��stub.dll����FixExeReloc�޸�Ŀ���ļ��ض�λ
//************************************
// ������:	ChangeReloc
// ����:	���ڶ�̬���ػ�ַ,��Ҫ��stub���ض�λ����(.reloc)�޸ĺ󱣴�,��PE�ض�λ��Ϣָ��ָ��õ�ַ�������Σ�
// ����ֵ:	void
// ����:	PCHAR pBuf ����stubdll���ڴ��׵�ַ
//************************************
void CPe::ChangeReloc(PCHAR pBuf)
{
	// ��λ����һ���ض�λ��
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + pBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	PIMAGE_DATA_DIRECTORY pRelocDir = (pNt->OptionalHeader.DataDirectory + 5);
	PIMAGE_BASE_RELOCATION pReloc =
		(PIMAGE_BASE_RELOCATION)(pRelocDir->VirtualAddress + pBuf);

	// ��ʼ�����ض�λ
	while(pReloc->SizeOfBlock != 0)
	{
		// �ض�λ�ʼ����,���䶨λ���ڴ�֮ǰ���15pb��  
		pReloc->VirtualAddress = (DWORD)(pReloc->VirtualAddress - 0x1000 + GetLastSectionRva());
		// ��λ����һ���ض�λ��
		pReloc = (PIMAGE_BASE_RELOCATION)((PCHAR)pReloc + pReloc->SizeOfBlock);
	}

	DWORD dwRelocRva = 0;
	DWORD dwRelocSize = 0;
	DWORD dwSectionAttribute = 0;
	while(TRUE)
	{
		if(!strcmp((char*)pSection->Name , ".reloc"))
		{
			dwRelocRva = pSection->VirtualAddress;
			dwRelocSize = pSection->SizeOfRawData;
			dwSectionAttribute = pSection->Characteristics;
			break;
		}
		pSection = pSection + 1;
	}

	// ��stubdll��.reloc��ӵ�PE�ļ������,����Ϊ.nreloc,���ظ����ε�Rva
	DWORD RelocRva = AddSection(".nreloc" , dwRelocRva + pBuf , dwRelocSize , dwSectionAttribute);

	// ���ض�λ��Ϣָ������ӵ�����
	PIMAGE_DOS_HEADER pExeDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pExeNt = (PIMAGE_NT_HEADERS)(pExeDos->e_lfanew + m_pNewBuf);
	pExeNt->OptionalHeader.DataDirectory[5].VirtualAddress = RelocRva;
	pExeNt->OptionalHeader.DataDirectory[5].Size = dwRelocSize;



}

//���Ҫ���һ��������,�����������ε�rva
DWORD CPe::GetNewSectionRva()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	PIMAGE_SECTION_HEADER pLastSection = pSection + pNt->FileHeader.NumberOfSections - 1;

	return pLastSection->VirtualAddress +
		CalcAlignment(pLastSection->Misc.VirtualSize , 0x1000);
}


//��ȡ���һ���ε�rva
DWORD CPe::GetLastSectionRva()
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	PIMAGE_SECTION_HEADER pLastSection = pSection + pNt->FileHeader.NumberOfSections - 1;

	return (DWORD)pLastSection;
}



#include "../aplib/aplib.h"
#pragma comment(lib,"..//aplib//aplib.lib")
//ѹ������ ѹ���ڼ�������֮��
void CPe::EnCompression(PPACKINFO & pPackInfo)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);

	//���ڼ�¼ѹ�����εĸ���
	pPackInfo->PackSectionNumber = 0;

	//1. ��ȡ�ļ�ͷ�Ĵ�С������ȡ����Դ��.rsrc���̱߳��ش洢.tls֮������ε��ļ��ܴ�С
	DWORD SecSizeWithOutResAndTls = 0;
	PIMAGE_SECTION_HEADER pSectionTmp_1 = pSection;	//��ʱ����ͷ��ַ
	BOOL flag = TRUE;		//����
	DWORD dwHeaderSize = 0;	//�ļ�ͷ��С

	for (size_t i = 0; i < pNt->FileHeader.NumberOfSections; i++)
	{
		//��ȡ��һ���ǿ����ε��ļ�ƫ�ƣ����ļ�ͷ��С
		if (flag && pSectionTmp_1->SizeOfRawData != 0)
		{
			dwHeaderSize = pSectionTmp_1->PointerToRawData;
			flag = FALSE;
		}
		//��ȡ�� rsrc/tls�˵��ܴ�С
		if (pSectionTmp_1->VirtualAddress != m_pResSectionRva &&
			pSectionTmp_1->VirtualAddress != m_pTlsSectionRva)
		{
			SecSizeWithOutResAndTls += pSectionTmp_1->SizeOfRawData;
		}
		pSectionTmp_1++;
	}


	//2. ��ȡҪѹ���Ķε��ڴ�

	//�����ڴ�
	PCHAR memWorked = new CHAR[SecSizeWithOutResAndTls];

	//�Ѿ��������ڴ��С
	DWORD dwCopySize = 0;

	//������Щ���ε��ڴ�
	PIMAGE_SECTION_HEADER pSectionTmp_2 = pSection;

	//����Ҫѹ�����ε��¿ռ�
	for (size_t i = 0; i < pNt->FileHeader.NumberOfSections; i++)
	{
		if (pSectionTmp_2->VirtualAddress != m_pResSectionRva &&
			pSectionTmp_2->VirtualAddress != m_pTlsSectionRva)
		{
			memcpy_s(memWorked + dwCopySize,
				pSectionTmp_2->SizeOfRawData,
				m_pNewBuf + pSectionTmp_2->PointerToRawData,
				pSectionTmp_2->SizeOfRawData);
			dwCopySize += pSectionTmp_2->SizeOfRawData;
		}
		pSectionTmp_2++;
	}

	//3. ѹ��
	long CompressedSize;
	PCHAR CompressData = Compress(memWorked, SecSizeWithOutResAndTls, CompressedSize);

	//4. ����.rsrc .tls�ε��ڴ�ռ�
	PCHAR resBuffer = new CHAR[m_ResSizeOfRawData];
	PCHAR tlsBuffer = new CHAR[m_TlsSizeOfRawData];
	memcpy_s(resBuffer, m_ResSizeOfRawData, m_ResPointerToRawData+m_pNewBuf,m_ResSizeOfRawData);
	memcpy_s(tlsBuffer, m_TlsSizeOfRawData, m_TlsPointerToRawData + m_pNewBuf, m_TlsSizeOfRawData);

	//5. ����ѹ����Ϣ����Ϣ�ṹ��
	
	//ԭĿ���ļ�PE��Ϣ
	PIMAGE_DOS_HEADER pOriDos = (PIMAGE_DOS_HEADER)m_pBuf;
	PIMAGE_NT_HEADERS pOriNt = (PIMAGE_NT_HEADERS)(pOriDos->e_lfanew + m_pBuf);
	PIMAGE_SECTION_HEADER pOriSection = IMAGE_FIRST_SECTION(pOriNt);

	for (int i = 0; i < pOriNt->FileHeader.NumberOfSections; i++)
	{
		if (pOriSection->VirtualAddress != m_pResSectionRva &&
			pOriSection->VirtualAddress != m_pTlsSectionRva)
		{
			//���ڻ�ȡѹ������������
			pPackInfo->PackSectionNumber++;
			//����ѹ������index
			pPackInfo->PackInfomation[pPackInfo->PackSectionNumber][0] = i;
			//����ѹ���������ļ���С
			pPackInfo->PackInfomation[pPackInfo->PackSectionNumber][1] = pOriSection->SizeOfRawData;
			//����ԭ���Ľ������ļ��е�ƫ�ƺʹ�СΪ0
			pOriSection->SizeOfRawData = 0;
			pOriSection->PointerToRawData = 0;
		}
		pOriSection++;
	}

	//6. �����¿ռ䣬ʹm_pNewBufָ��������m_pBuf�ļ�ͷ����
	m_FileSize = dwHeaderSize + m_ResSizeOfRawData + m_TlsSizeOfRawData;//�ļ�ͷ��С + res��С + Tls��С
	//��m_pNewBufָ����
	m_pNewBuf = new CHAR[m_FileSize];
	//�޸�res�ε�����ͷ
	pOriSection = IMAGE_FIRST_SECTION(pOriNt);	//��λ������ͷ
	if (m_ResSectionIndex < m_TlsSectionIndex)	//��Դ����Tls��ǰ��
	{
		//��ԭ��Դ���ļ�ƫ������Ϊ�ļ�ͷ��β����Ϊ��һ������
		pOriSection[m_ResSectionIndex].PointerToRawData = dwHeaderSize;
		//��ԭTLs���ļ�ƫ�����õ��������Դ������
		pOriSection[m_TlsSectionIndex].PointerToRawData = dwHeaderSize + m_ResSizeOfRawData;

		//�ȸ���ԭ�ļ����ݵ��¿ռ�
		memcpy_s(m_pNewBuf, dwHeaderSize, m_pBuf, dwHeaderSize);
		//������Դ�����ݵ�����ͷλ�ã���Դ�γ�Ϊ��һ������
		memcpy_s(m_pNewBuf + dwHeaderSize, m_ResSizeOfRawData, resBuffer, m_ResSizeOfRawData);
		//����TLS�Σ��ڶ�������
		memcpy_s(m_pNewBuf + dwHeaderSize + m_ResSizeOfRawData, m_TlsSizeOfRawData,
			tlsBuffer, m_TlsSizeOfRawData);
	}
	else if (m_ResSectionIndex > m_TlsSectionIndex)	//��Դ��˳����TLS�κ���
	{
		pOriSection[m_TlsSectionIndex].PointerToRawData = dwHeaderSize;
		pOriSection[m_ResSectionIndex].PointerToRawData = dwHeaderSize + m_TlsSizeOfRawData;
		memcpy_s(m_pNewBuf, dwHeaderSize, m_pBuf, dwHeaderSize);
		memcpy_s(m_pNewBuf + dwHeaderSize, m_TlsSizeOfRawData, tlsBuffer, m_TlsSizeOfRawData);
		memcpy_s(m_pNewBuf + dwHeaderSize + m_TlsSizeOfRawData
			, m_ResSizeOfRawData, resBuffer, m_ResSizeOfRawData);
	}
	else
	{
		//û����Դ�κ�TLS ֻ�踴��ԭ���ļ����¿ռ�
		memcpy_s(m_pNewBuf, dwHeaderSize, m_pBuf, dwHeaderSize);
	}

	delete[] m_pBuf;
	m_pBuf = m_pNewBuf;

	//�������
	pPackInfo->packSectionRva = AddSection(".compres", CompressData, CompressedSize, 0xC0000040);
	pPackInfo->packSectionSize = CalcAlignment(CompressedSize, 0x200);

	//7. ���.compres��
	delete[] memWorked;
	free(CompressData);
	delete[] resBuffer;
}


 
 //����ѹ����
 PCHAR CPe::Compress(PVOID pSource, IN long InLength, OUT long & OutLength)
 {
 	PCHAR CompressData=NULL;		//����ѹ�����ݵĿռ�
 	PCHAR workmem= NULL;			//Ϊ���ѹ����Ҫʹ�õĿռ�
 
	if ((CompressData = (PCHAR)malloc(aP_max_packed_size(InLength))) == NULL ||		//���ѹ���ļ��Ŀռ��С
		(workmem = (PCHAR)malloc(aP_workmem_size(InLength))) == NULL)			//�����ռ��С
	{
		return NULL;
	}

	//����aPsafe_packѹ������,����ѹ����Ŀռ��С
	OutLength = aPsafe_pack(pSource, CompressData, InLength, workmem, NULL, NULL);
	if (OutLength == APLIB_ERROR) return NULL;

	free(workmem);
	workmem = NULL;

 	return CompressData;
 }
 


 //�޸�Tls��
 BOOL CPe::ModifyTlsTable(PPACKINFO & pPackInfo)
 {
	 PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	 PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + m_pNewBuf);


	 if (pNt->OptionalHeader.DataDirectory[9].VirtualAddress == 0)	//���������TLS��
	 {
		 pPackInfo->bIsTlsUserful = FALSE;
		 return FALSE;
	 }
	 else
	 {
		 pPackInfo->bIsTlsUserful = TRUE;

		 //��ȡTls���ڴ��еĵ�ַ RVA + ���ڴ��е���ʼ��ַ
		 PIMAGE_TLS_DIRECTORY32 g_lpTlsDir = (PIMAGE_TLS_DIRECTORY32)
			 (RvaToOffset(pNt->OptionalHeader.DataDirectory[9].VirtualAddress) + m_pNewBuf);
		 //��ȡTLS������RVA  AddressOfIndex��VA�� - ���ػ�ַ
		 DWORD TlsIndexRVA = g_lpTlsDir->AddressOfIndex - pNt->OptionalHeader.ImageBase;
		 //��ȡTLS�������ļ�ƫ��
		 DWORD TlsIndexOffset = RvaToOffset(TlsIndexRVA);
		 //����Tls������ֵ
		 pPackInfo->TlsIndex = 0;
		 if (TlsIndexOffset != -1)
		 {
			 //TLS���� = ƫ�� + �ļ���ַ
			 pPackInfo->TlsIndex = *(DWORD*)(TlsIndexOffset + m_pNewBuf);
		 }
		 //����Ŀ�����TLS����Ϣ
		 m_StartOfDataAddress = g_lpTlsDir->StartAddressOfRawData;	//Դ���ݵ���ʼ��ַ  ��VA)
		 m_EndOfDataAddress = g_lpTlsDir->EndAddressOfRawData;		//Դ���ݵ���ֹ��ַ	��VA)
		 m_CallBackFuncAddress = g_lpTlsDir->AddressOfCallBacks;		//����TLS������λ�� ��VA)

																		//��TLS�ص�����RVA���õ�������Ϣ�ṹ��
		 pPackInfo->TlsCallbackFuncRva = m_CallBackFuncAddress;
		 return TRUE;
	 }
 }



 //************************************
 // ������:	SetTls
 // ����:	
 // ����ֵ:	void
 // ����:	DWORD NewSectionRva  ����ӵ�15pb���ε�rva
 // ����:	PCHAR pStubBuf       stubdll���ڴ��ָ��
 // ����:	DWORD pPackInfo		 ������Ϣ�ṹ����׵�ַ,PackInfo�б�����tlsRva
 //************************************
 void CPe::SetTls(DWORD NewSectionRva, PCHAR pStubBuf, PPACKINFO pPackInfo)
 {
	 PIMAGE_DOS_HEADER pStubDos = (PIMAGE_DOS_HEADER)pStubBuf;
	 PIMAGE_NT_HEADERS pStubNt = (PIMAGE_NT_HEADERS)(pStubDos->e_lfanew + pStubBuf);

	 PIMAGE_DOS_HEADER pPeDos = (PIMAGE_DOS_HEADER)m_pNewBuf;
	 PIMAGE_NT_HEADERS pPeNt = (PIMAGE_NT_HEADERS)(pPeDos->e_lfanew + m_pNewBuf);

	 //0 ��peĿ¼��9ָ��stub��tls��
	 pPeNt->OptionalHeader.DataDirectory[9].VirtualAddress =
		 (pStubNt->OptionalHeader.DataDirectory[9].VirtualAddress - 0x1000) + NewSectionRva;
	 pPeNt->OptionalHeader.DataDirectory[9].Size =
		 pStubNt->OptionalHeader.DataDirectory[9].Size;

	 PIMAGE_TLS_DIRECTORY32  pITD =
		 (PIMAGE_TLS_DIRECTORY32)(RvaToOffset(pPeNt->OptionalHeader.DataDirectory[9].VirtualAddress) + m_pNewBuf);
	 // ��ȡ�����ṹ����tlsIndex��rva
	 DWORD indexRva = ((DWORD)pPackInfo - (DWORD)pStubBuf + 4) - 0x1000 + NewSectionRva + pPeNt->OptionalHeader.ImageBase;
	 pITD->AddressOfIndex = indexRva;
	 pITD->StartAddressOfRawData = m_StartOfDataAddress;
	 pITD->EndAddressOfRawData = m_EndOfDataAddress;

	 // ������ȡ��tls�Ļص�����,������Ϣ�ṹ���д���tls�ص�����ָ��,��stub��ǵĹ������ֶ�����tls,����tls�ص�����ָ�����û�ȥ
	 pITD->AddressOfCallBacks = 0;

	 m_pBuf = m_pNewBuf;
 }
