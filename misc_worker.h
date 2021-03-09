#pragma once
#include "m3u8_task.h"
#include "m3u8_repo.h"
#include <base/memory/singleton.h>
class MiscWorker : public nbase::SupportWeakCallback
{
public:
	SINGLETON_DEFINE(MiscWorker);
public:
	MiscWorker() {

	}
	~MiscWorker() {

	}
	void get_all_task();
	void add_task_item(models::M3u8Task task);
	void add_task_item_and_download(models::M3u8Task task);
	void delete_task_item(models::M3u8Task task);
public:
	void delay_task_process_download();
private:
	//folder_name,task
	std::map<std::string, models::M3u8Task> _map_download_queue;
};

