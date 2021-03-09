#include "pch.h"
#include "uc_task_item.h"
#include <boost/format.hpp>
#include "task_manager_worker.h"
#include <boost/foreach.hpp>
#include "misc_worker.h"
//#include <boost/filesystem.hpp>
UCTaskItem::UCTaskItem()
{
	
}
UCTaskItem::~UCTaskItem()
{
}

void UCTaskItem::InitSubControls(models::M3u8Task& taskItemModel, std::function<void(std::string, models::M3u8Task&)> OnClickBubble)
{
	_OnClickBubble = OnClickBubble;
	if (taskItemModel._id == 0) {		
		_task_item_model._content = taskItemModel._content;
	}
	_task_item_model._id = taskItemModel._id;
	_task_item_model._url = taskItemModel._url;// set_request_url(taskItemModel.get_request_url());
	_task_item_model._title = taskItemModel._title;// set_title(taskItemModel.get_title());
	_task_item_model._status = taskItemModel._status;
	_task_item_model._folder_name = taskItemModel._folder_name;
	//_task_item_model->_content = taskItemModel._content;// set_context(taskItemModel.get_context());
	_task_item_model.count = taskItemModel.count;
	_task_item_model.count_complete = taskItemModel.count_complete;
	_task_item_model.count_downloading = taskItemModel.count_downloading;
	_task_item_model.count_error = taskItemModel.count_error;

	// 查找 Item 下的控件	 
	label_title_ = dynamic_cast<ui::Label*>(FindSubControl(L"label_title"));
	label_info = dynamic_cast<ui::Label*>(FindSubControl(L"label_info"));
	label_progress = dynamic_cast<ui::Label*>(FindSubControl(L"label_progress"));
	label_title_->SetText(nbase::UTF8ToUTF16(taskItemModel._title));

	boost::format fmt = boost::format("[%1%]/[%2%]/[%3%]/[%4%]")% taskItemModel.count
		%taskItemModel.count_complete
		%taskItemModel.count_error
		%taskItemModel.count_downloading;
	std::string info = fmt.str();
	label_progress->SetText(nbase::UTF8ToUTF16(info));

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
	RefreshCtrls(taskItemModel);
}
void UCTaskItem::RefreshCtrls(models::M3u8Task& _task_item_model) {
	if (_task_item_model._status == models::M3u8Task::Status::Saved) {
		if (_task_item_model._id == 0) {
			//未保存
			label_info->SetText(L"未保存");
			btn_download_save->SetEnabled(true);
			btn_save->SetEnabled(true);
			btn_play->SetEnabled(false);
			btn_edit->SetEnabled(false);
			btn_del_->SetEnabled(false);
		}
		if (_task_item_model._id != 0) {
			label_info->SetText(L"已保存");
			btn_download_save->SetEnabled(true);
			btn_save->SetText(L"已保存");
			btn_save->SetEnabled(false);
			btn_play->SetEnabled(false);
			btn_edit->SetEnabled(true);
			btn_del_->SetEnabled(true);
		}
		label_info->SetText(L"等待下载");
	}else if (_task_item_model._status == models::M3u8Task::Status::WaitingForDownload) {
		label_info->SetText(L"等待下载");
		btn_download_save->SetEnabled(true);
		btn_download_save->SetText(L"暂停");
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(false);		 

	}else if (_task_item_model._status == models::M3u8Task::Status::Downloading) {
		label_info->SetText(L"下载中");
		btn_download_save->SetEnabled(true);
		btn_download_save->SetText(L"暂停");
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(false);
	}
	else if (_task_item_model._status == models::M3u8Task::Status::DownloadComplete) {
		label_info->SetText(L"下载完成");
		btn_download_save->SetEnabled(false);
		btn_download_save->SetText(L"下载");
		if (_task_item_model.count_error != 0) {
			btn_download_save->SetEnabled(true);
			label_info->SetText(L"点击下载失败Ts");
		}
		btn_play->SetEnabled(true);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(true);
	}
	else if (_task_item_model._status == models::M3u8Task::Status::Pause) {
		label_info->SetText(L"已暂停");
		btn_download_save->SetEnabled(true);
		btn_download_save->SetText(L"下载");
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(true);
	}
	if (_task_item_model._status != models::M3u8Task::Status::Saved) {
		btn_save->SetVisible(false);
	}
	if (_task_item_model.count != 0 && _task_item_model.count_complete == _task_item_model.count && _task_item_model.count_error == 0) {
		btn_download_save->SetVisible(false);
	}
	boost::format fmt = boost::format("[%1%]/[%2%]/[%3%]/[%4%]") %
		_task_item_model.count%
		_task_item_model.count_complete%
		_task_item_model.count_error%
		_task_item_model.count_downloading;

	std::string info8 = fmt.str();
	std::wstring info = nbase::UTF8ToUTF16(info8);

	label_progress->SetText(info);
}
bool UCTaskItem::OnClick(ui::EventArgs* args)
{
	ui::Button* btn = (ui::Button*)args->pSender;
	std::wstring btnName = btn->GetName();
	if (btnName == L"btn_download_save") {
		btn_download_save->SetEnabled(false);
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(false);
		btn_save->SetEnabled(false);
		btn->SetEnabled(false);
		btn->SetText(L"处理中");
		nbase::ThreadManager::PostTask(kThreadMisc, nbase::Bind(&MiscWorker::add_task_item_and_download, MiscWorker::GetInstance(), _task_item_model));
	}
	else if (btnName == L"btn_save") {
		btn_download_save->SetEnabled(false);
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(false);
		btn_save->SetEnabled(false);
		btn->SetEnabled(false);
		btn->SetText(L"保存中");
		nbase::ThreadManager::PostTask(kThreadMisc, nbase::Bind(&MiscWorker::add_task_item, MiscWorker::GetInstance(), _task_item_model));
	}
	else if (btnName == L"btn_play") {
		this->_OnClickBubble(nbase::UTF16ToUTF8(btnName),_task_item_model);
	}
	else if (btnName == L"btn_edit") {
		//btn_download_save->SetEnabled(true);
	}
	else if (btnName == L"btn_del") {
		btn_download_save->SetEnabled(false);
		btn_save->SetEnabled(false);
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(false);
		btn->SetEnabled(false);
		btn->SetText(L"删除中");
		nbase::ThreadManager::PostTask(kThreadMisc, nbase::Bind(&MiscWorker::delete_task_item, MiscWorker::GetInstance(), _task_item_model));
		
	}
	return true;
}
 
void UCTaskItem::UpdateControls(models::M3u8Task& taskItemModel) {
		
	_task_item_model._id = taskItemModel._id;
	_task_item_model._url = taskItemModel._url;// set_request_url(taskItemModel.get_request_url());
	_task_item_model._title = taskItemModel._title;// set_title(taskItemModel.get_title());
	_task_item_model._status = taskItemModel._status;
	_task_item_model._folder_name = taskItemModel._folder_name;
	_task_item_model._content = taskItemModel._content;// set_context(taskItemModel.get_context());
	_task_item_model.count = taskItemModel.count;
	_task_item_model.count_complete = taskItemModel.count_complete;
	_task_item_model.count_downloading = taskItemModel.count_downloading;
	_task_item_model.count_error = taskItemModel.count_error;
	this->RefreshCtrls(_task_item_model);
}

//int res = db_.Query("CREATE TABLE [M3u8Task] ([id] INTEGER  NOT NULL PRIMARY KEY,[status] INTEGER DEFAULT '0' NULL,[title] VARCHAR(255)  NULL,[request_url] VARCHAR(1024)  NULL,[folder_name] vARCHAR(255)  NULL,[aria2_request_status] INTEGER DEFAULT '0' NULL,[aria2_download_status] INTEGER DEFAULT '0' NULL,[context] TEXT  NULL)");
		//	//int rowcount = models::M3u8Task::GetCountBy(db_, _task_item_model->get_title());