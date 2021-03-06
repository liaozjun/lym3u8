#include "pch.h"
#include "http_server_runner.h"
#
void HttpServerRunner::RunMongooseLoop(std::string url)
{
	_url = url;
	_hInst = LoadLibrary(L"lymg.dll");
	//根据函数名获取dll地址
	typedef void(*StartMgoo)(const char*,const char*);
	StartMgoo Sub = (StartMgoo)GetProcAddress(_hInst, "StartMgoo");
	 
	std::string rootdir = nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory());
	rootdir += "m3u8";
	Sub(rootdir.data(), _url.data());
}
void HttpServerRunner::StopMongooseLoop()
{
	typedef void(*EndMgoo)();
	EndMgoo Sub = (EndMgoo)GetProcAddress(_hInst, "EndMgoo");
	Sub();
}