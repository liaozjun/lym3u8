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
	/*std::string GetUrl() {
		return _task_item_model->_url;
	}*/
	//void InserTask();
	/*models::M3u8Task::Status GetTaskStatus() {
		return _task_item_model->_status;
	} */
	void UpdateControls(models::M3u8Task& t);
	
	
	bool equalUrl(std::string url) {
		return _task_item_model._url == url;
	}
private:
	std::function<void(std::string, models::M3u8Task&) > _OnClickBubble;
	bool OnClick(ui::EventArgs* args);
	void RefreshCtrls(models::M3u8Task& _task_item_model); 
private:
	ui::ListBox* 	list_box_;
	ui::Label*		label_title_;
	ui::Label*		label_progress;
	ui::Label*		label_info;

	ui::Button*		btn_download_save;
	ui::Button*		btn_save;
	ui::Button*		btn_play;
	ui::Button*		btn_edit;
	ui::Button*		btn_del_;

	/*int64 _task_item_id;
	std::string _url;
	std::string _title;
	std::string _content;
	std::string _folder_name;*/
	models::M3u8Task _task_item_model;
	//std::unique_ptr<models::M3u8Task> _task_item_model;
};
