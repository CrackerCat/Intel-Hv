#include"window.h"
#include"include/PDBSDK.h"
#include"service_hook.h"
#include"include/vector.hpp"
#include"settings.h"

#ifdef HIDE_WINDOW

using std::vector;

extern "C" 
{
	extern ULONG_PTR Win32kfullBase;
	extern ULONG_PTR Win32kbaseBase;
}

extern std::vector<MM_SESSION_SPACE*> vSesstionSpace;

struct tagWND
{
	HWND hwnd;
	char pad[0xa0];
	wchar_t* WindowName;
};
static_assert(offsetof(tagWND, WindowName) == 0xa8);

static ULONG_PTR gpKernelHandleTable = Win32kbaseBase + OffsetgpKernelHandleTable;

void Window::Init()
{
	pfMiAttachSession(vSesstionSpace[0]);

	ASSERT(MmIsAddressValid((PVOID)gpKernelHandleTable));

	gpKernelHandleTable = *(ULONG_PTR*)gpKernelHandleTable;
	pfMiDetachProcessFromSession(1);

}

//
//һ��user ��2��session һ����session1����һ����session0
//�û�������session 1
//
void AttackWindowTable()
{
	pfMiAttachSession(vSesstionSpace[1]);

	for (int hwnd = 1; hwnd < 0xffff; hwnd++)
	{
		static tagWND* wnd_ptr = nullptr;
		if (MmIsAddressValid((PVOID)(hwnd * 24 + gpKernelHandleTable)))
			wnd_ptr = *(tagWND**)(hwnd * 24 + gpKernelHandleTable);
		else
			continue;

		if (MmIsAddressValid(wnd_ptr) && MmIsAddressValid(wnd_ptr->WindowName)) {
#if 0
			Log("HWND %llx,WIndow Name %ws\n", wnd_ptr->hwnd, wnd_ptr->WindowName);
#endif
			if (!wcscmp(wnd_ptr->WindowName, L"x64dbg"))
			{
				memset(wnd_ptr->WindowName, 0, 2);
			}

			if (!wcscmp(wnd_ptr->WindowName, L"x32dbg"))
			{
				memset(wnd_ptr->WindowName, 0, 2);
			}


		}
	}

	//
	//�����DriverDispatch����������������Ļ���detach�����������DriverEntry�ǵ��þͲ���
	//���ǲ�detach����Ҳûʲô���� 
	//

	//pfMiDetachProcessFromSession(1);
}


#endif // HIDE_WINDOW

