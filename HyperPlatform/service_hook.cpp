#include"service_hook.h"
#include"include/stdafx.h"
#include"include/vector.hpp"
#include"include/exclusivity.h"
#include"include/write_protect.h"
#include"kernel-hook/khook/hde/hde.h"
#include"include/handle.h"
#include"include/PDBSDK.h"
#include"common.h"

extern "C"
{
#include"kernel-hook/khook/khook/hk.h"
extern ULONG_PTR KernelBase;
extern ULONG_PTR PspCidTable;
extern ULONG_PTR Win32kfullBase;
extern ULONG Win32kfullSize;
}

#define PAGE_FAULT_READ 0

const char* test_process = "Dbgview.exe";

const char* target_process = "csgo.exe";

struct myUnicodeString : public UNICODE_STRING
{
	~myUnicodeString() {}
};

myUnicodeString ux64dbg = RTL_CONSTANT_STRING(L"x64dbg");
myUnicodeString uxdbg64 = RTL_CONSTANT_STRING(L"xdbg64");
myUnicodeString ux32dbg = RTL_CONSTANT_STRING(L"x32dbg");
myUnicodeString uxdbg32 = RTL_CONSTANT_STRING(L"xdbg32");
myUnicodeString uCheatEngine73 = RTL_CONSTANT_STRING(L"Cheat Engine 7.3");

//���и���ݷ�ʽ�������Ҫ���ԵĻ�
//
//#define target_process test_process
//



using std::vector; 
vector<ServiceHook> vServcieHook;
hde64s gIns;

static bool init = false;
vector<MM_SESSION_SPACE*> vSesstionSpace;

static vector<myUnicodeString> vHideWindow;

#pragma optimize( "", off )
void ServiceHook::Construct()
{

	if (!pfMiGetSystemRegionType)
		pfMiGetSystemRegionType = (MiGetSystemRegionTypeType)(KernelBase + OffsetMiGetSystemRegionType);

	if (!pfMmGetSessionById)
		pfMmGetSessionById = (MmGetSessionByIdType)(KernelBase + OffsetMmGetSessionById);
	
	//����һ��bool init������ֻ��Ҫһ�εĳ�ʼ������
	if (!init) {

		vHideWindow.push_back(ux64dbg);
		vHideWindow.push_back(uxdbg64);
		vHideWindow.push_back(ux32dbg);
		vHideWindow.push_back(uxdbg32);
		vHideWindow.push_back(uCheatEngine73);



		pfMiAttachSession = (MiAttachSessionType)(KernelBase + OffsetMiAttachSession);
		pfMiDetachProcessFromSession = (MiDetachProcessFromSessionType)(KernelBase + OffsetMiDetachProcessFromSession);

		//
		for (int i = 0; i < 1000; i++)
		{
			PEPROCESS Process = pfMmGetSessionById(i);
			if (Process)
			{
				SystemSesstionSpace = *(MM_SESSION_SPACE**)((ULONG_PTR)Process + 0x400);
				vSesstionSpace.push_back(SystemSesstionSpace);

			}
		}
		//



#ifdef DBG
		for (auto session : vSesstionSpace)
		{
			Log("Session ID %d\n", session->SessionId);

			//��ʼ��ַ�ͽ�����ַ������һ��
			Log("PagedPoolStart %p\n", session->PagedPoolStart);
			Log("PagedPoolEnd %p\n", session->PagedPoolEnd);
		}
#endif // DBG

		

		init = true;
	}
#if 0
	for (int i = 8;;)
	{
		//�ų�system���̺�idle����
		PEPROCESS process = (PEPROCESS)GetObject10((PHANDLE_TABLE10)PspCidTable, i);
		//����session
		SystemSesstionSpace = *(MM_SESSION_SPACE**)((ULONG_PTR)process + 0x400);

		//�������session
		if (SystemSesstionSpace) {
			ULONG32 SystemSesstionID = SystemSesstionSpace->SessionId;
			PVOID SystemSesstionPoolStart = SystemSesstionSpace->PagedPoolStart;
			PVOID SystemSesstionPoolEnd = SystemSesstionSpace->PagedPoolEnd;
		}
	}
#endif

	for (auto Session : vSesstionSpace)
	{
		//if (this->fp.GuestVA == PVOID(Win32kfullBase + OffsetNtUserFindWindowEx))
		//{
			//this->isWin32Hook = true;
		//}
		if (this->fp.GuestVA > (PVOID)Win32kfullBase && this->fp.GuestVA < (PVOID)(Win32kfullBase + Win32kfullSize))
		{
			this->isWin32Hook = true;
		}

	}

	if (!this->DetourFunc || !this->TrampolineFunc || !this->fp.GuestVA)
	{
		Log("DetourFunc or TrampolineFunc or fp.GuestVA is null!\n");
		Log("DetourFunc %p\nTrampolineFunc %p\nfp.GuestVA %p\n",
			this->DetourFunc, this->TrampolineFunc, this->fp.GuestVA);
		return;
	}


#if 1
	/**
	* Q:MmGetPhysicalAddress(NtCreateThread)���ص������ַΪ0
	*   MmGetPhysicalAddress(NtCreateThreadEx)���ص������ַ��Ϊ0
	* 
	* A:
	* 1: kd> u ntcreatethread
	nt!NtCreateThread:
	fffff807`1a6948f0 ??              ???


0: kd> !pte fffff807`1a6948f0
										   VA fffff8071a6948f0
PXE at FFFFFCFE7F3F9F80    PPE at FFFFFCFE7F3F00E0    PDE at FFFFFCFE7E01C698    PTE at FFFFFCFC038D34A0
contains 0000000001208063  contains 0000000001209063  contains 0000000001217063  contains 0000FEF300002064
pfn 1208      ---DA--KWEV  pfn 1209      ---DA--KWEV  pfn 1217      ---DA--KWEV  not valid
																				  PageFile:  2
																				  Offset: def3
																				  Protect: 3 - ExecuteRead
	��ʵ����ķ�ҳ�����ִ�����ϵͳ���ڴ�ѹ����

	*/


#endif

	//���ָ����������ҳ�Ŀ�ʼ��
	auto tmp = (PVOID)(((ULONG_PTR)(this->fp).GuestVA >> 12) << 12);
	//GuestPAΪGuestVA���ҳ����ʼ�������ַ
	//GuestVA�����ʼ�����ܸı�
#if 0
	if(this->fp.GuestVA == (PVOID)0xfffff80725d288f0)
	DbgBreakPoint();
#endif
	//
	//���pte.vaildΪ0��MmGetPhysicalAddress����0
	//
	this->fp.GuestPA = MmGetPhysicalAddress(tmp);
#if 1 //�ṩ��ҳ����֧��
	if (!pfMmAccessFault)
		pfMmAccessFault = (MmAccessFaultType)(KernelBase + OffsetMmAccessFault);

	if (!this->fp.GuestPA.QuadPart)
	{

#if 0  //��õ�ǰ��ַ���ڵ�POOL_TYPE
		auto RegionType = pfMiGetSystemRegionType((ULONG_PTR)this->fp.GuestVA);
#endif
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

		//
		//MmAccessFault��ҳ������������������ҳ�濽����ʱ��ϵͳ������ǻ�����
		//MmAccessFault��ʱ�򻹻�����,���绻NtUserFindWindowEx����ҳ
		//
		
		//pfMmAccessFault(PAGE_FAULT_READ, nullptr, this->fp.GuestVA, KernelMode);
		
		//���ṩһ�λ���
		//this->fp.GuestPA = MmGetPhysicalAddress(tmp);
	}
#endif
	this->fp.PageContent = ExAllocatePoolWithQuota(NonPagedPool, PAGE_SIZE);

	//����ԭ��ҳ��,������ʱ��ÿ��ҳ������ݶ�һ����д��ʱ���Ҫ�ֱ�д�ˡ�
	if (this->isWin32Hook) {
		pfMiAttachSession(vSesstionSpace[0]);
	}
	memcpy(this->fp.PageContent, tmp, PAGE_SIZE);


	

	this->fp.PageContentPA = MmGetPhysicalAddress(this->fp.PageContent);

	//�ȴ�����ϵͳ�����ǻ�ҳ�꣬�����ڻ�ȡ
	this->fp.GuestPA = MmGetPhysicalAddress(tmp);

	if (!fp.GuestPA.QuadPart || !fp.PageContentPA.QuadPart)
	{
		HYPERPLATFORM_COMMON_DBG_BREAK();
		Log("MmGetPhysicalAddress error %s %d\n",__func__,__LINE__);
		return;
	}

	auto exclusivity = ExclGainExclusivity();
	//
	//
	//mov rax,xx
	//jmp rax
	//
	//
	static char hook[] = { 0x48,0xB8,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0xFF,0xE0 };
	size_t CodeLength = 0;
	while (CodeLength < 12)
	{
		HdeDisassemble((void*)((ULONG_PTR)(this->fp.GuestVA) + CodeLength), &gIns);
		CodeLength += gIns.len;
	}
	this->HookCodeLen = CodeLength;
	/*
	* 1.����һ����̬�ڴ�(Orixxxxx)���溯����ͷ����12���ֽ�,���ü���һ��jmpxxx���ֽڣ���ΪOri����jmp��ȥ
	* 2.Ȼ���޸ĺ�����ͷΪmove rax,xx jump rax,xx
	* 3.
	*/

	*(this->TrampolineFunc) = ExAllocatePoolWithTag(NonPagedPool, CodeLength + 14, 'a');
	if (!*(this->TrampolineFunc))
	{
		Log("ExAllocatePoolWithTag failed ,no memory!\n");
		return;
	}
	
	memcpy(*(this->TrampolineFunc), this->fp.GuestVA, CodeLength);
	static char hook2[] = { 0xff,0x25,0,0,0,0,1,1,1,1,1,1,1,1 };
	ULONG_PTR jmp_return = (ULONG_PTR)this->fp.GuestVA + CodeLength;
	memcpy(hook2 + 6, &jmp_return, 8);
	memcpy((void*)((ULONG_PTR)(*(this->TrampolineFunc)) + CodeLength), hook2, 14);
	auto irql = WPOFFx64();

	PVOID* Ptr = &this->DetourFunc;
	memcpy(hook + 2, Ptr, 8);
	memcpy((PVOID)this->fp.GuestVA, hook, sizeof(hook));

	WPONx64(irql);
	

	ExclReleaseExclusivity(exclusivity);

	if (this->isWin32Hook)
		pfMiDetachProcessFromSession(1);

	this->isEverythignSuc = true;

}
#pragma optimize( "", on )

void ServiceHook::Destruct()
{
	if (!this->isEverythignSuc)
		return;

#if 0
	NTSTATUS Status = HkRestoreFunction((this->fp).GuestVA, this->TrampolineFunc);
	if (!NT_SUCCESS(Status)) {
		Log("HkRestoreFunction Failed %x\n", Status);
		return;
	}
#endif

	if (this->isWin32Hook)
		pfMiAttachSession(vSesstionSpace[0]);



	//
	//�ⲿ�ִ���������÷�ҳ���ڴ滻��������������߳��л��ͻ�������
	//
	if(KeGetCurrentIrql()>= DISPATCH_LEVEL)
		KeLowerIrql(APC_LEVEL);
	char tmp[1];
	memcpy(tmp, this->fp.GuestVA, 1);
	//
	//û������ҳ,��������
	//
	if (!MmIsAddressValid(this->fp.GuestVA))
	{
		KeBugCheck(0x11111110);
	}
	//
	auto Exclu = ExclGainExclusivity();
	auto irql = WPOFFx64();
	memcpy(this->fp.GuestVA, *(this->TrampolineFunc), this->HookCodeLen);
	WPONx64(irql);
	ExclReleaseExclusivity(Exclu);

	if (this->isWin32Hook)
		pfMiDetachProcessFromSession(1);

	ExFreePool(*(this->TrampolineFunc));

}

void AddServiceHook(PVOID HookFuncStart, PVOID Detour, PVOID *TramPoline)
{
	ServiceHook tmp;
	tmp.DetourFunc = Detour;
	tmp.fp.GuestVA = HookFuncStart;
	tmp.TrampolineFunc = TramPoline;
	tmp.Construct();
	vServcieHook.push_back(tmp);
}

void RemoveServiceHook()
{
	for (auto& hook : vServcieHook)
	{
		hook.Destruct();
	}
}

//
//hook example
//

NTSTATUS DetourNtOpenProcess(
	PHANDLE            ProcessHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PCLIENT_ID         ClientId
)
{
#ifdef DBG

	static int once = 0;
	if (!(once++))
		Log("%s\n", __func__);

#endif // DBG

	return OriNtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
}

NTSTATUS DetourNtCreateFile(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	PLARGE_INTEGER     AllocationSize,
	ULONG              FileAttributes,
	ULONG              ShareAccess,
	ULONG              CreateDisposition,
	ULONG              CreateOptions,
	PVOID              EaBuffer,
	ULONG              EaLength
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("%s\n", __func__);
#endif // DBG

	return OriNtCreateFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength);
}

//
//�����û�̬�ڴ�д��
//
NTSTATUS DetourNtWriteVirtualMemory(
	IN HANDLE ProcessHandle,
	OUT PVOID BaseAddress,
	IN CONST VOID* Buffer,
	IN SIZE_T BufferSize,
	OUT PSIZE_T NumberOfBytesWritten OPTIONAL
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("%s\n", __func__);
#endif // DBG

	NTSTATUS Status STATUS_UNSUCCESSFUL;

	PEPROCESS Process = NULL;
	Status = ObReferenceObjectByHandle(ProcessHandle,
		PROCESS_VM_WRITE,
		*PsProcessType,
		UserMode,
		(PVOID*)&Process,
		NULL);

	if (Process)
	{
		unsigned char* Image = PsGetProcessImageFileName(Process);

		if (!strcmp((const char*)Image, target_process))
		{
			Log("[%s]\nBaseAddress %llx BufferSize %llx\n",__func__, BaseAddress, BufferSize);
		}








	}
	return OriNtWriteVirtualMemory(
		ProcessHandle,
		BaseAddress,
		Buffer,
		BufferSize,
		NumberOfBytesWritten);
}

NTSTATUS DetourNtCreateThreadEx(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID lpStartAddress,
	IN PVOID lpParameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID lpBytesBuffer)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("%s\n", __func__);
#endif // DBG

	NTSTATUS Status STATUS_UNSUCCESSFUL;

	PEPROCESS Process = NULL;
	Status = ObReferenceObjectByHandle(ProcessHandle,
		PROCESS_VM_WRITE,
		*PsProcessType,
		UserMode,
		(PVOID*)&Process,
		NULL);

	if (Process)
	{
		unsigned char* Image = PsGetProcessImageFileName(Process);
		const unsigned char* Image2 = PsGetProcessImageFileName(IoGetCurrentProcess());

		if (!strcmp((const char*)Image, target_process) && strcmp((const char*)Image2, target_process))
		{
			Log("[csgo]\nThreadProcedure %llx\n", lpStartAddress);

			if (lpParameter)
				Log("lpParameter value is %llx\n", lpParameter);


		}


	}
	

	return OriNtCreateThreadEx(
		hThread, 
		DesiredAccess,
		ObjectAttributes, 
		ProcessHandle,
		lpStartAddress,
		lpParameter,
		Flags,
		StackZeroBits, 
		SizeOfStackCommit, 
		SizeOfStackReserve,
		lpBytesBuffer);
}

//����ڴ����
NTSTATUS DetourNtAllocateVirtualMemory(
	HANDLE    ProcessHandle,
	PVOID* BaseAddress,
	ULONG_PTR ZeroBits,
	PSIZE_T   RegionSize,
	ULONG     AllocationType,
	ULONG     Protect
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("%s\n", __func__);
#endif // DBG

	NTSTATUS Status STATUS_UNSUCCESSFUL;

	PEPROCESS Process = NULL;
	Status = ObReferenceObjectByHandle(ProcessHandle,
		PROCESS_VM_WRITE,
		*PsProcessType,
		UserMode,
		(PVOID*)&Process,
		NULL);

	if (Process)
	{
		unsigned char* Image = PsGetProcessImageFileName(Process);
		const unsigned char* Image2 = PsGetProcessImageFileName(IoGetCurrentProcess());
		if (!strcmp((const char*)Image, target_process) && strcmp((const char*)Image2, target_process))
		{
			Log("[%s]\nAlloc RegionSize %p\n", __func__, *RegionSize);
		}


	}






	return OriNtAllocateVirtualMemory(
		ProcessHandle,
		BaseAddress,
		ZeroBits,
		RegionSize,
		AllocationType,
		Protect);
}


NTSTATUS DetourNtCreateThread(
	OUT PHANDLE ThreadHandle,
	IN  ACCESS_MASK DesiredAccess,
	IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
	IN  HANDLE ProcessHandle,
	OUT PCLIENT_ID ClientId,
	IN  PCONTEXT ThreadContext,
	IN  PVOID InitialTeb,
	IN  BOOLEAN CreateSuspended
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("%s\n", __func__);
#endif // DBG


	NTSTATUS Status STATUS_UNSUCCESSFUL;

	PEPROCESS Process = NULL;
	Status = ObReferenceObjectByHandle(ProcessHandle,
		PROCESS_VM_WRITE,
		*PsProcessType,
		UserMode,
		(PVOID*)&Process,
		NULL);

	if (Process)
	{
		unsigned char* Image = PsGetProcessImageFileName(Process);
		const unsigned char* Image2 = PsGetProcessImageFileName(IoGetCurrentProcess());

		if (!strcmp((const char*)Image, target_process) && strcmp((const char*)Image2, target_process))
		{
			Log("[%s]\nThreadProcedure %llx\n",__func__ ,ThreadContext->Rcx);
		}


	}

	return OriNtCreateThread(
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		ClientId,
		ThreadContext,
		InitialTeb,
		CreateSuspended
	);
}
/*
hook win32k ������ע������

00 ffffdd07`2186d780 fffff807`255490fb     nt!MiAttachSession+0x6c
01 ffffdd07`2186d7d0 fffff807`255c7675     nt!MiTrimOrAgeWorkingSet+0x77b
02 ffffdd07`2186d8b0 fffff807`255c6056     nt!MiProcessWorkingSets+0x255
03 ffffdd07`2186da60 fffff807`25620c17     nt!MiWorkingSetManager+0xaa
04 ffffdd07`2186db20 fffff807`254dfa45     nt!KeBalanceSetManager+0x147
05 ffffdd07`2186dc10 fffff807`2565cb8c     nt!PspSystemThreadStartup+0x55
06 ffffdd07`2186dc60 00000000`00000000     nt!KiStartSystemThread+0x1c


*/

HWND DetourNtUserFindWindowEx(  // API FindWindowA/W, FindWindowExA/W
	IN HWND hwndParent,
	IN HWND hwndChild,
	IN PUNICODE_STRING pstrClassName,
	IN PUNICODE_STRING pstrWindowName)
{
#if 0
	if(pstrWindowName->Buffer)
		Log("[%s]%ws\n", __func__, pstrWindowName->Buffer);

	if (pstrClassName->Buffer)
		Log("[%s]%ws\n",__func__ ,pstrClassName->Buffer);
#endif // DBG

	for (auto window : vHideWindow) {
		if (!RtlCompareUnicodeString(pstrWindowName, &window, 1))
			return 0;
	}
	return OriNtUserFindWindowEx(hwndParent, hwndChild, pstrClassName, pstrWindowName);
}


NTSTATUS DetourNtDeviceIoControlFile(
	_In_ HANDLE FileHandle,
	_In_opt_ HANDLE Event,
	_In_opt_ PIO_APC_ROUTINE ApcRoutine,
	_In_opt_ PVOID ApcContext,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_ ULONG IoControlCode,
	_In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_writes_bytes_opt_(OutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength
)
{
	NTSTATUS status;
	FILE_OBJECT* FileObject;
	status = ObReferenceObjectByHandle(FileHandle, FILE_ALL_ACCESS, *IoFileObjectType, KernelMode, (PVOID*)&FileObject, NULL);
	
	
	if (NT_SUCCESS(status)) {

		unsigned char* Image = PsGetProcessImageFileName(IoGetCurrentProcess());

		if (!strcmp((const char*)Image, "vssadmin.exe") || !strcmp((const char*)Image, "wmic.exe"))
		{
			POBJECT_NAME_INFORMATION p = NULL;
			status = IoQueryFileDosDeviceName(FileObject, &p);

			if (p && NT_SUCCESS(status)) {

			Log("[service]%wZ  [ioctl-code] %x\n", p->Name, IoControlCode);
			
			}



			ExFreePool(p);
		}

	}


	return OriNtDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer,
		InputBufferLength, OutputBuffer, OutputBufferLength);
}