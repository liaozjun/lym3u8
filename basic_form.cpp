#include "pch.h"
#include "basic_form.h"
#include "uc_task_item.h"
#include "m3u8_repo.h"
#include "third_party/jsoncpp/include/json/json.h"

const std::wstring BasicForm::kClassName = L"lym3u8_form";

BasicForm::BasicForm()
{
	http_server_runner_.reset(new HttpServerRunner());
}
BasicForm::~BasicForm()
{
}
ui::Control* BasicForm::CreateControl(const std::wstring& pstrClass)
{
	// 扫描 XML 发现有名称为 CefControl 的节点，则创建一个 ui::CefControl 控件
	if (pstrClass == L"CefControl")
	{
		if (nim_comp::CefManager::GetInstance()->IsEnableOffsetRender())
			return new nim_comp::CefControl;
		else
			return new nim_comp::CefNativeControl;
	}

	return NULL;
}
std::wstring BasicForm::GetSkinFolder()
{
	return L"basic";
}

std::wstring BasicForm::GetSkinFile()
{
	return L"basic.xml";
}

std::wstring BasicForm::GetWindowClassName() const
{
	return kClassName;
}

void BasicForm::InitWindow()
{
	cef_control_ = dynamic_cast<nim_comp::CefControlBase*>(FindControl(L"cef_control"));
	cef_control_dev_ = dynamic_cast<nim_comp::CefControlBase*>(FindControl(L"cef_control_dev"));
	cef_control_->AttachDevTools(cef_control_dev_);
	if (!nim_comp::CefManager::GetInstance()->IsEnableOffsetRender()) {
		cef_control_dev_->SetVisible(false);
	}
	cef_control_->LoadURL(L"http://www.yongjiuzy1.com/?m=vod-play-id-33050-src-1-num-65.html");
	//cef_control_->LoadURL(nbase::win32::GetCurrentModuleDirectory() + L"dist/index.html");
	list_box_ = dynamic_cast<ui::ListBox*>(FindControl(L"list"));
	task_loading_ = dynamic_cast<ui::HBox*>(FindControl(L"task_loading"));

	cef_control_->AttachTitleChange(nbase::Bind(&BasicForm::OnTitleChanged, this, std::placeholders::_1));
	cef_control_->AttachM3u8LoadComplete(nbase::Bind(&BasicForm::M3u8LoadComplete, this,std::placeholders::_1,std::placeholders::_2));

	/*this->task_loading_->SetVisible(false);
	this->list_box_->SetVisible(true);

	/*models::M3u8Task mt;
	mt._title = "ddd";
	this->AddTask(mt);*/
	this->TaskListLoading(true);
	nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&BasicForm::kThreadTaskProcess_GetAllTask,this));
	//nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&TaskProcessRunner::testrun, task_process_runner_.get()));	
//	nbase::ThreadManager::PostTask(kThreadHttpServer, nbase::Bind(&HttpServerRunner::RunMongooseLoop, http_server_runner_.get()));
}
void BasicForm::OnTitleChanged(std::wstring title)
{
	_title = title;
}

void BasicForm::AddUCTaskItem(models::M3u8Task& mt) {
	UCTaskItem* item = new UCTaskItem;
	ui::GlobalManager::FillBoxWithCache(item, L"basic/task_item.xml");
	item->InitSubControls(mt);
	list_box_->AddAt(item, 0);
}

void BasicForm::TaskListLoading(bool isloading) {
	this->task_loading_->SetVisible(isloading);
	this->list_box_->SetVisible(!isloading);
}

void BasicForm::M3u8LoadComplete(std::string url, std::string json_parms) 
{
	models::M3u8Task m(nbase::UTF16ToUTF8(_title), url, json_parms);
	this->AddUCTaskItem(m);
} 

LRESULT BasicForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	http_server_runner_->StopMongooseLoop();

	nim_comp::CefManager::GetInstance()->PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MY_PIPE_BUFFER_SIZE 1024
void StartProcess()
{
	//初始化管道
	HANDLE hPipeRead;
	HANDLE hPipeWrite;
	SECURITY_ATTRIBUTES saOutPipe;
	::ZeroMemory(&saOutPipe, sizeof(saOutPipe));
	saOutPipe.nLength = sizeof(SECURITY_ATTRIBUTES);
	saOutPipe.lpSecurityDescriptor = NULL;
	saOutPipe.bInheritHandle = TRUE;
	if (CreatePipe(&hPipeRead, &hPipeWrite, &saOutPipe, MY_PIPE_BUFFER_SIZE))
	{
		PROCESS_INFORMATION processInfo;
		::ZeroMemory(&processInfo, sizeof(processInfo));
		STARTUPINFO startupInfo;
		::ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startupInfo.hStdOutput = hPipeWrite;
		startupInfo.hStdError = hPipeWrite;
		startupInfo.wShowWindow = SW_SHOW;
		std::wstring cmdline = _T("E:\\NimDuilib\\lym3u8\\Debug\\static\\aria2c.exe --conf-path=E:\\NimDuilib\\lym3u8\\Debug\\static\\aria2.conf --check-certificate=false");

		if (::CreateProcessW(NULL, (LPWSTR)(cmdline.c_str()),
			NULL,  // process security
			NULL,  // thread security
			TRUE,  //inheritance
			0,     //no startup flags
			NULL,  // no special environment
			NULL,  //default startup directory
			&startupInfo,
			&processInfo))
		{
			if (WAIT_TIMEOUT != WaitForSingleObject(processInfo.hProcess, 3000))
			{
				DWORD dwReadLen = 0;
				DWORD dwStdLen = 0;
				if (PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwReadLen, NULL) && dwReadLen > 0)
				{
					char szPipeOut[MY_PIPE_BUFFER_SIZE];
					::ZeroMemory(szPipeOut, sizeof(szPipeOut));
					if (ReadFile(hPipeRead, szPipeOut, dwReadLen, &dwStdLen, NULL))
					{
						// 输出值
						int k = 0;
					}
					int a = 1;
				}
			}
		}
		if (processInfo.hProcess)
		{
			CloseHandle(processInfo.hProcess);
		}
		if (processInfo.hThread)
		{
			CloseHandle(processInfo.hThread);
		}
	}
	CloseHandle(hPipeRead);
	CloseHandle(hPipeWrite);
}
void BasicForm::kThreadTaskProcess_GetAllTask( ) {
	ndb::SQLiteDB db_;
	bool result = db_.Open("./lygg", "", ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		std::list<models::M3u8Task> list;
		repos::M3u8Repo::QueryAll(db_, list);
		for (auto it = list.begin(); it != list.end(); it++)// auto mt : list)
		{
			this->AddUCTaskItem(*it);
		}
	}
	db_.Close();
	//Sleep(10000);
	this->TaskListLoading(false);	
	
	Json::Value root;
	Json::Reader jr;
	jr.parse("{\"id\": \"qwer1\",\"jsonrpc\" : \"2.0\",	\"result\" : \"2bc4d00f31295d4a\"}", root);
	std::string result1 = root["result1"].asString();
	//StartProcess();
	this->RunAria2();

	nbase::ThreadManager::PostDelayedTask(kThreadTaskProcess, nbase::Bind(&BasicForm::kThreadTaskProcess_DelayTask_ProcessDownload, this)
		, nbase::TimeDelta::FromMilliseconds(1000 * 3));
}
void BasicForm::RunAria2() {
	std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
	DWORD processId = GetCurrentProcessId();//当前进程id
	//--stop-with-process=
	std::wstring pid = nbase::Uint64ToString16(processId);
	std::wstring cmdline = nbase::StringPrintf(L"%sstatic\\aria2c.exe --conf-path=%sstatic\\aria2.conf --check-certificate=false --stop-with-process=%s",
		theme_dir.data(), theme_dir.data(), pid.data()); 
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	//隐藏掉可能出现的cmd命令窗口
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;//SW_HIDE
	ZeroMemory(&pi, sizeof(pi));
	BOOL bRet = ::CreateProcess(
		NULL,//启动程序路径名 
		(LPWSTR)(cmdline.c_str()), //参数（当exeName为NULL时，可将命令放入参数前） 
		NULL, //使用默认进程安全属性 
		NULL, //使用默认线程安全属性 
		FALSE,//句柄不继承 
		0, //使用正常优先级 
		NULL, //使用父进程的环境变量 
		NULL, //指定工作目录 
		&si, //子进程主窗口如何显示 
		&pi); //用于存放新进程的返回信息 

		//NULL, (LPWSTR)(cmdline.c_str()), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	DWORD ecode = 0;
	if (bRet == 0)
	{
		ecode = GetLastError();
	}
}
void BasicForm::kThreadTaskProcess_DelayTask_ProcessDownload() 
{
	LOG(INFO) << "kThreadTaskProcess_DelayTask_ProcessDownload";
	int length = list_box_->GetCount();
	bool isDownloading = false;
	for (int i = 0; i < length; i++)
	{
		UCTaskItem* ctrl = (UCTaskItem*)(list_box_->GetItemAt(i));
		if (ctrl->GetTaskStatus() == models::M3u8Task::Status::Downloading) {
			isDownloading = true;
			ctrl->ProcessDownloading();
			break;
		}
	}
	if (!isDownloading) {
		for (int i = 0; i < length; i++)
		{
			UCTaskItem* ctrl = (UCTaskItem*)(list_box_->GetItemAt(i));
			if (ctrl->GetTaskStatus() == models::M3u8Task::Status::WaitingForDownload) {
				ctrl->ProcessWaitingForDownload();
				break;
			}
		}
	}

	nbase::ThreadManager::PostDelayedTask(kThreadTaskProcess, nbase::Bind(&BasicForm::kThreadTaskProcess_DelayTask_ProcessDownload, this)
		, nbase::TimeDelta::FromMilliseconds(1000 * 5));
}