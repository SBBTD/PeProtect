// Pack.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "string"
#include "PE.h"
#include "..\\Stub\\Stub.h"
using namespace std;

//�ӿ�
extern "C" _declspec(dllexport)
bool Pack(PCHAR pPath)
{
	bool ret = false;
	//1 ��stub.dll���뵽�ڴ�
	HMODULE hStub = LoadLibrary(L"..\\Debug\\Stub.dll");
	//3 ���ڴ����ҵ���stub.dllͨѶ�� g_PackInfo
	PPACKINFO pPackInfo = (PPACKINFO)GetProcAddress(hStub , "g_PackInfo");
	CPe obj;
	ret = obj.ReadTargetFile(pPath, pPackInfo);
	if (!ret)
	{
		return ret;
	}

	//��ȡTLS��Ϣ
	BOOL bTlsUseful = obj.ModifyTlsTable(pPackInfo);

	// �Դ���ν��м���
	obj.Encryption();

	//ѹ������
	obj.EnCompression(pPackInfo);

	//2 ��ȡstub.dll���ڴ��С�ͽ���ͷ(Ҳ����Ҫ������ͷ��)
	PIMAGE_DOS_HEADER pStubDos = (PIMAGE_DOS_HEADER)hStub;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pStubDos->e_lfanew + (PCHAR)hStub);
	DWORD dwImageSize = pNt->OptionalHeader.SizeOfImage;
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);//stub.dllʹ��Release���룬��һ������Ϊ.text ��Debug���룺.textbss

	
	//4. ���üӿ���Ϣ======

	// ��ȡĿ�����OEP
	pPackInfo->TargetOepRva = obj.GetOepRva();
	// ��ȡĿ�������ػ�ַIamgebase
	pPackInfo->ImageBase = obj.GetImageBase();
	// ��ȡĿ������ض�λ��rva�͵�����rva
	pPackInfo->ImportTableRva = obj.GetImportTableRva();
	pPackInfo->RelocRva = obj.GetRelocRva();

	//5 ���Stub.dllģ����Start��������������ַ��VA-Stub.dll��ַ
	DWORD dwStartRva = (DWORD)pPackInfo->StartAddress - (DWORD)hStub;
	// ---���޸�������ͨѶ�ṹ�������֮���ٶ�dll�����ڴ濽��---
	//6 ����ֱ���ڱ��������޸Ļ�Ӱ�����,���Խ�dll����һ�ݵ�pStubBuf
	PCHAR pStubBuf = new CHAR[dwImageSize];
	memcpy_s(pStubBuf , dwImageSize , (PCHAR)hStub , dwImageSize);

	//7 �޸�dll�ļ��ض�λ,����ڶ�������Ӧ�ô���Stub.dllģ���ַhStub,��Ϊ����dll����ʱ�ض�λ������
	obj.FixDllRloc(pStubBuf, (PCHAR)hStub);

	//8 ��stub.dll�Ĵ����.text���ΪĿ������������
	DWORD NewSectionRva = obj.AddSection(
		".15PB" ,									//������
		pSection->VirtualAddress + pStubBuf,		//���ε�ַ
		pSection->SizeOfRawData,					//���δ�С
		pSection->Characteristics					//��������
	);
	
	obj.SetTls(NewSectionRva, (PCHAR)hStub, pPackInfo);
	
	//=================�ض�λ���====================
	// ����ѡ��ȥ���ض�λ
	//obj.CancleRandomBase();
	// ���߽�stub���ض�λ����ճ�������,���ض�λ��ָ��֮,������֮ǰҲ����FixDllRloc,ʹ����Ӧ�µ�PE�ļ�
	obj.ChangeReloc(pStubBuf);
	
	//9 ��Ŀ������OEP����Ϊstub�е�start����

	DWORD dwChazhi = (dwStartRva - pSection->VirtualAddress);
	DWORD dwNewOep = (dwChazhi + NewSectionRva);
	obj.SetNewOep(dwNewOep);
	
	// ����ÿ�����ο�д
	obj.SetMemWritable();

	// ��IAT���м���
	obj.ChangeImportTable();

	FreeLibrary(hStub);
	//10 ������ļ�
	string savePath = pPath;
	savePath = savePath + "_pack.exe";
	obj.SaveNewFile((char*)savePath.c_str());

	return ret;
}
