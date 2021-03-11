// lym3u8.cpp : 定义应用程序的入口点。
//

#include "pch.h"
#include "framework.h"
#include "lym3u8.h"
#include "main.h"

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
// 控制是否使用离屏渲染，当为 true  时将使用默认窗口阴影方案，离屏渲染模式下窗口有 WS_EX_LAYERED 属性
// 当为 false 时因使用了真窗口模式不支持带有 WS_EX_LAYERED 属性窗口，所以使用外置窗口阴影方案，需要在 xml 中将窗口 shadowattached 属性设置为 false
const bool kEnableOffsetRender = false;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 将 bin\\cef 目录添加到环境变量，这样可以将所有 CEF 相关文件放到该目录下，方便管理
	// 在项目属性->连接器->输入，延迟加载 nim_libcef.dll
	nim_comp::CefManager::GetInstance()->AddCefDllToPath();

	HRESULT hr = ::OleInitialize(NULL);
	if (FAILED(hr))
		return 0;

	// 初始化 CEF
	CefSettings settings;
	if (!nim_comp::CefManager::GetInstance()->Initialize(nbase::win32::GetCurrentModuleDirectory() + L"cef_temp\\", settings, kEnableOffsetRender))
	{
		return 0;
	}

	// 创建主线程
	MainThread thread;

	// 执行主线程循环
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

	// 清理 CEF
	nim_comp::CefManager::GetInstance()->UnInitialize();

	::OleUninitialize();

	return 0;
}

