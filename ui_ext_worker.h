#pragma once
#include "basic_form.h"
#include <base/memory/singleton.h>
#include "uc_task_item.h"
class UiExtWorker : public nbase::SupportWeakCallback
{
public:
	SINGLETON_DEFINE(UiExtWorker);
public:
	UiExtWorker() {//BasicForm* bf
		//_basicForm = bf;
	}
	~UiExtWorker() {

	}
	void Init(BasicForm* bf) {
		_basicForm = bf;
	}
	void get_all_task_callback(std::list<models::M3u8Task> tasks);
	void add_task_item_callback(models::M3u8Task task);
	void add_task_item_and_download_callback(models::M3u8Task task);
	void delete_task_item_callback(models::M3u8Task task);
	void add_uitaskitem(std::string title, std::string url, std::string json_parms);
	void update_uitaskitem(models::M3u8Task task);
private:
	BasicForm* _basicForm;
	std::map<std::string, UCTaskItem*> _map_ucTaskItem;
};

