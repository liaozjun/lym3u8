#pragma once
#include "m3u8_task.h"
class UCTaskItem : public ui::ListContainerElement
{
public:
	UCTaskItem();
	~UCTaskItem();

	// 提供外部调用来初始化 item 数据
	void InitSubControls(models::M3u8Task& taskItemDto);
	//std::string GetTitle() {
	//	return _task_item_model->_title;// get_title();
	//}
	//void InserTask();
	models::M3u8Task::Status GetTaskStatus() {
		return _task_item_model->_status;
	} 
	void ProcessDownloading();
	void ProcessWaitingForDownload();
private:
	bool OnClick(ui::EventArgs* args);
	void RefreshCtrls();
	void kThreadTaskProcess_InserTask();
	void kThreadTaskProcess_InserTaskAndDownload();
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
