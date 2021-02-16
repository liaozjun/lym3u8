#include "pch.h"
#include "uc_task_item.h"
#include "m3u8_repo.h"
UCTaskItem::UCTaskItem()
{
	
}


UCTaskItem::~UCTaskItem()
{
}

void UCTaskItem::InitSubControls(models::M3u8Task& taskItemModel)
{
	_task_item_model = std::make_unique<models::M3u8Task>();
	_task_item_model->_id = taskItemModel._id;
	_task_item_model->_url = taskItemModel._url;// set_request_url(taskItemModel.get_request_url());
	_task_item_model->_title = taskItemModel._title;// set_title(taskItemModel.get_title());
	_task_item_model->_status = taskItemModel._status;
	_task_item_model->_folder_name = taskItemModel._folder_name;
	_task_item_model->_content = taskItemModel._content;// set_context(taskItemModel.get_context());

	// 查找 Item 下的控件	 
	label_title_ = dynamic_cast<ui::Label*>(FindSubControl(L"label_title"));
	label_progress = dynamic_cast<ui::Label*>(FindSubControl(L"label_progress"));
	label_title_->SetText(nbase::UTF8ToUTF16(_task_item_model->_title));// get_title()));
	label_progress->SetText(L"[0/10]");
	// 绑定删除任务处理函数
	btn_download_save = dynamic_cast<ui::Button*>(FindSubControl(L"btn_download_save"));
	btn_save = dynamic_cast<ui::Button*>(FindSubControl(L"btn_save"));
	btn_play = dynamic_cast<ui::Button*>(FindSubControl(L"btn_play"));
	btn_edit = dynamic_cast<ui::Button*>(FindSubControl(L"btn_edit"));
	btn_del_ = dynamic_cast<ui::Button*>(FindSubControl(L"btn_del"));

	btn_download_save->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_save->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_play->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_edit->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_del_->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	RefreshCtrls();
}
void UCTaskItem::RefreshCtrls() {
	if (_task_item_model->_id != 0) {
		btn_save->SetVisible(false);
	}
	else {
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(false);
	}
	if (_task_item_model->_status == models::M3u8Task::Status::WaitingForDownload) {
		btn_download_save->SetText(L"等待");
		btn_download_save->SetEnabled(false);
		btn_download_save->SetState(ui::ControlStateType::kControlStateDisabled);
		this->Invalidate();
	}
}
bool UCTaskItem::OnClick(ui::EventArgs* args)
{
	ui::Button* btn = (ui::Button*)args->pSender;
	std::wstring btnName = btn->GetName();
	if (btnName == L"btn_download_save") {
		nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&UCTaskItem::kThreadTaskProcess_InserTaskAndDownload, this));
	}
	else if (btnName == L"btn_save") {
		btn_save->SetVisible(false);
		nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&UCTaskItem::kThreadTaskProcess_InserTask, this));
	}
	else if (btnName == L"btn_play") {

	}
	else if (btnName == L"btn_edit") {

	}
	else if (btnName == L"btn_del") {
		ui::ListBox* parent = dynamic_cast<ui::ListBox*>(this->GetParent());
		return parent->Remove(this);
	}
	return true;
}

void UCTaskItem::ProcessDownloading() {

}

void UCTaskItem::ProcessWaitingForDownload() {
	_task_item_model->_details_ts.clear();

	ndb::SQLiteDB db_;
	bool result = db_.Open("./lygg", "", ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		bool res = repos::M3u8Repo::GetTaskDetails(db_, _task_item_model->_id, _task_item_model->_details_ts);
		if (res) {
			_task_item_model->SendAria2();

			this->_task_item_model->_status = models::M3u8Task::Status::Downloading;
			//repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
	}
	db_.Close();
}

void UCTaskItem::kThreadTaskProcess_InserTask() {
	
	ndb::SQLiteDB db_;
	bool result = db_.Open("./lygg", "", ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{//int res = db_.Query("CREATE TABLE [M3u8Task] ([id] INTEGER  NOT NULL PRIMARY KEY,[status] INTEGER DEFAULT '0' NULL,[title] VARCHAR(255)  NULL,[request_url] VARCHAR(1024)  NULL,[folder_name] vARCHAR(255)  NULL,[aria2_request_status] INTEGER DEFAULT '0' NULL,[aria2_download_status] INTEGER DEFAULT '0' NULL,[context] TEXT  NULL)");
	//	//int rowcount = models::M3u8Task::GetCountBy(db_, _task_item_model->get_title());
		repos::M3u8Repo::Insert(db_, *_task_item_model);
		RefreshCtrls();
	}
	db_.Close();
}

void UCTaskItem::kThreadTaskProcess_InserTaskAndDownload()
{
	ndb::SQLiteDB db_;
	bool result = db_.Open("./lygg", "", ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{//int res = db_.Query("CREATE TABLE [M3u8Task] ([id] INTEGER  NOT NULL PRIMARY KEY,[status] INTEGER DEFAULT '0' NULL,[title] VARCHAR(255)  NULL,[request_url] VARCHAR(1024)  NULL,[folder_name] vARCHAR(255)  NULL,[aria2_request_status] INTEGER DEFAULT '0' NULL,[aria2_download_status] INTEGER DEFAULT '0' NULL,[context] TEXT  NULL)");
	//	//int rowcount = models::M3u8Task::GetCountBy(db_, _task_item_model->get_title());
		if (_task_item_model->_id == 0) {
			repos::M3u8Repo::Insert(db_, *_task_item_model);
		}
		_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
		repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
	}
	db_.Close();
	RefreshCtrls();

}