// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"

#include "targetver.h"
#include "Resource.h"
// C runtime header
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
//#include <tchar.h>

// base header
#include "base/base.h"

// duilib
#include "duilib/UIlib.h"

// ui components
#include "ui_components/ui_components.h"
#include "ui_components/ui_cef_control.h"
enum ThreadId
{
	kThreadUI,
	kThreadMisc,
	kThreadHttpServer,
};
#ifdef _DEBUG
#pragma comment(lib,"libcurld.lib")
#pragma comment(lib,"jsoncpp_d.lib")
#pragma comment(lib,"wxsqlite3_d.lib")
#pragma comment(lib, "db_d.lib")
//#pragma comment(lib,"libeay32.lib")
#else
#pragma comment(lib,"jsoncpp.lib")
#pragma comment(lib,"libcurl.lib")
#pragma comment(lib,"wxsqlite3.lib")
#pragma comment(lib,"db.lib")
#endif

bool TestBindPort(u_short port);
std::string GetGuid();
#endif //PCH_H
