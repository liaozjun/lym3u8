#include "pch.h"
#include "basic_form.h"
#include "uc_task_item.h"
#include "m3u8_repo.h"
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
	//cef_control_->LoadURL(L"https://cn.pornhub.com/view_video.php?viewkey=ph5ddf2ce189a7d");
	cef_control_->LoadURL(nbase::win32::GetCurrentModuleDirectory() + L"dist/index.html");
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
	nbase::ThreadManager::PostDelayedTask(kThreadTaskProcess, nbase::Bind(&BasicForm::kThreadTaskProcess_DelayTask_ProcessDownload, this)
		, nbase::TimeDelta::FromMilliseconds(1000 * 3));
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
		, nbase::TimeDelta::FromMilliseconds(1000 * 3));
}