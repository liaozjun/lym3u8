#pragma once
#include "m3u8_task.h"
#include "m3u8_repo.h"
class UCTaskItem : public ui::ListContainerElement
{
public:
	UCTaskItem();
	~UCTaskItem();
	
	// 提供外部调用来初始化 item 数据
	void InitSubControls(models::M3u8Task& taskItemDto,std::function<void (std::string, models::M3u8Task&)> OnClickBubble);
	std::string GetUrl() {
		return _task_item_model->_url;
	}
	//void InserTask();
	models::M3u8Task::Status GetTaskStatus() {
		return _task_item_model->_status;
	} 
	void ProcessDownloading();
	void ProcessWaitingForDownload();
	std::string RequestAria2(std::string& cmd);
	std::string RequestAria2TellStatus(models::M3u8Ts& ts);
	std::string RequestAria2AddUri(models::M3u8Ts& ts);
	bool equalUrl(std::string url) {
		return _task_item_model->_url == url;
	}
private:
	std::function<void(std::string, models::M3u8Task&) > _OnClickBubble;
	bool OnClick(ui::EventArgs* args);
	void RefreshCtrls();
	void kThreadTaskProcess_InserTask();
	void kThreadTaskProcess_InserTaskAndDownload();
	bool ProcessTsDownload(ndb::SQLiteDB& db);
private:
	ui::ListBox* 	list_box_;
	ui::Label*		label_title_;
	ui::Label* label_progress;

	ui::Button*		btn_download_save;
	ui::Button*		btn_save;
	ui::Button*		btn_play;
	ui::Button*		btn_edit;
	ui::Button*		btn_del_;

	std::unique_ptr<models::M3u8Task> _task_item_model;
};
