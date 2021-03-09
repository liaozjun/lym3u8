#include "pch.h"
#include "basic_form.h"
#include "uc_task_item.h"
#include "m3u8_repo.h"
#include "third_party/jsoncpp/include/json/json.h"
#include <boost/format.hpp>
#include "ui_ext_worker.h"
#include "misc_worker.h"
const std::wstring BasicForm::kClassName = L"lym3u8_form";
std::string http_server_url = "http://localhost:8000/";

BasicForm::BasicForm()
{
	http_server_runner_.reset(new HttpServerRunner());

	//task_manager_worker.reset(new TaskManagerWorker());
 
	UiExtWorker::GetInstance()->Init(this);
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
#ifdef _DEBUG
	cef_control_dev_ = dynamic_cast<nim_comp::CefControlBase*>(FindControl(L"cef_control_dev"));
	cef_control_->AttachDevTools(cef_control_dev_);
	if (!nim_comp::CefManager::GetInstance()->IsEnableOffsetRender()) {
		cef_control_dev_->SetVisible(false);
	}
#endif // _DEBUG

	//cef_control_->LoadURL(L"https://cn.pornhub.com/view_video.php?viewkey=ph5d681e304a483");
	cef_control_->LoadURL(nbase::win32::GetCurrentModuleDirectory() + L"dist/index.html");
	list_box_ = dynamic_cast<ui::ListBox*>(FindControl(L"list"));
	task_loading_ = dynamic_cast<ui::HBox*>(FindControl(L"task_loading"));
	btn_navigate = dynamic_cast<ui::Button*>(FindControl(L"btn_navigate"));
	btn_refresh = dynamic_cast<ui::Button*>(FindControl(L"btn_refresh"));
	btn_home = dynamic_cast<ui::Button*>(FindControl(L"btn_home"));
	edit_url = dynamic_cast<ui::RichEdit*>(FindControl(L"edit_url"));
	edit_url->AttachReturn(nbase::Bind(&BasicForm::EditUrlReturn, this, std::placeholders::_1));

	btn_navigate->AttachClick(nbase::Bind(&BasicForm::OnClick, this, std::placeholders::_1));	

	cef_control_->AttachTitleChange(nbase::Bind(&BasicForm::OnTitleChanged, this, std::placeholders::_1));
	cef_control_->AttachM3u8LoadComplete(nbase::Bind(&BasicForm::M3u8LoadComplete, this,std::placeholders::_1,std::placeholders::_2));
	cef_control_->AttachLoadEnd(nbase::Bind(&BasicForm::OnLoadEnd, this, std::placeholders::_1));

	this->TaskListLoading(true); 
	nbase::ThreadManager::PostTask(kThreadMisc, nbase::Bind(&MiscWorker::get_all_task, MiscWorker::GetInstance()));


	u_short port = 8000;
	for (int i = 0; i < 1000; i++) {
		port = port + i;
		if (TestBindPort(port)) {
			break;
		}
	}
	boost::format fmt = boost::format("http://localhost:%1%/")%port;
	http_server_url = fmt.str();
	nbase::ThreadManager::PostTask(kThreadHttpServer, nbase::Bind(&HttpServerRunner::RunMongooseLoop, http_server_runner_.get(), http_server_url));
}
bool BasicForm::OnClick(ui::EventArgs* args) {
	ui::Button* btn = (ui::Button*)args->pSender;
	std::wstring btnName = btn->GetName();
	if (btnName == L"btn_navigate") {
		std::wstring url = edit_url->GetText();
		cef_control_->resetGotM3u8();
		cef_control_->LoadURL(url);	
	}
	return true;
}
bool BasicForm::EditUrlReturn(ui::EventArgs* args)
{
	std::wstring url = edit_url->GetText();
	cef_control_->resetGotM3u8();
	cef_control_->LoadURL(url);
	return true;
}
void BasicForm::OnTitleChanged(std::wstring title)
{
	_title = title;
}

void BasicForm::OnClickBubble(std::string action, models::M3u8Task& task)
{
	if (action == "btn_play") {
		cur_url = http_server_url + task._folder_name + "/index.m3u8";
		cef_control_->LoadURL(nbase::win32::GetCurrentModuleDirectory() + L"dist/index.html");
	}
	//list_box_->se
}
void BasicForm::OnLoadEnd(int httpStatusCode)
{
	cef_control_->resetGotM3u8();
	cef_control_->RegisterCppFunc(L"GetCurrPalyUrl", ToWeakCallback([this](const std::string& params, nim_comp::ReportResultFunction callback) {
		//nim_comp::Toast::ShowToast(nbase::UTF8ToUTF16(params), 3000, GetHWND());
		//callback(true, R"({ "message": "Success." })");
		std::string url = cur_url;
		boost::format fmt = boost::format(R"({"url":"%1%"})") % url;
		std::string result = fmt.str();
		callback(true, result);
	}));
}

UCTaskItem* BasicForm::AddUCTaskItem(models::M3u8Task& mt) {
	UCTaskItem* item = new UCTaskItem;
	ui::GlobalManager::FillBoxWithCache(item, L"basic/task_item.xml");
	item->InitSubControls(mt, std::bind(&BasicForm::OnClickBubble, this, std::placeholders::_1,std::placeholders::_2));
	list_box_->AddAt(item, 0);
	return item;
}

void BasicForm::TaskListLoading(bool isloading) {
	this->task_loading_->SetVisible(isloading);
	this->list_box_->SetVisible(!isloading);
}

void BasicForm::M3u8LoadComplete(std::string url, std::string json_parms) 
{	
	auto it = url.find("localhost");
	if (it != url.npos) {
		return;
	}
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		models::M3u8Task task;
		bool f = repos::M3u8Repo::GetM3u8Task(db_, url, task);
		if (task._id == 0) 
		{

			int length = list_box_->GetCount();
			bool isexists = false;
			for (int i = 0; i < length; i++)
			{
				UCTaskItem* ctrl = (UCTaskItem*)(list_box_->GetItemAt(i));
				if (ctrl->equalUrl(url)) {
					isexists = true;
					break;
				}
			}
			if (!isexists) 
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::add_uitaskitem, UiExtWorker::GetInstance(), nbase::UTF16ToUTF8(_title),url,json_parms));
				 
			}
			else {
				nim_comp::Toast::ShowToast(L"该URL已存在", 3000, GetHWND());
			}
		}
		else {
			nim_comp::Toast::ShowToast(L"该URL已存在", 3000, GetHWND());
		}
	}
	db_.Close();
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