#include "pch.h"
#include "ui_ext_worker.h"

void UiExtWorker::get_all_task_callback(std::list<models::M3u8Task> tasks) 
{	 
	for (auto it = tasks.begin(); it != tasks.end(); it++)// auto mt : list)
	{
		UCTaskItem* pti = _basicForm->AddUCTaskItem(*it);
		_map_ucTaskItem.insert(std::make_pair((*it)._folder_name, pti));
	}
	_basicForm->TaskListLoading(false);
}
void UiExtWorker::add_task_item_callback(models::M3u8Task task)
{
	auto it = _map_ucTaskItem.find(task._folder_name);
	if (it != _map_ucTaskItem.end())
	{
		UCTaskItem* pti = it->second;
		pti->UpdateControls(task);
	}
}
void UiExtWorker::add_task_item_and_download_callback(models::M3u8Task task) {
	auto it = _map_ucTaskItem.find(task._folder_name);
	if (it != _map_ucTaskItem.end())
	{
		UCTaskItem* pti = it->second;
		pti->UpdateControls(task);
	}
}

void UiExtWorker::delete_task_item_callback(models::M3u8Task task)
{
	auto it = _map_ucTaskItem.find(task._folder_name);
	if (it != _map_ucTaskItem.end())
	{	
		ui::ListBox* parent = dynamic_cast<ui::ListBox*>(it->second->GetParent());
		parent->Remove(it->second);
		_map_ucTaskItem.erase(it->first);		
	}	
}
void UiExtWorker::update_uitaskitem(models::M3u8Task task)
{
	void* p = &task;
	auto it = _map_ucTaskItem.find(task._folder_name);
	if (it != _map_ucTaskItem.end())
	{
		UCTaskItem* pti = it->second;
		pti->UpdateControls(task);
	}
}

void UiExtWorker::add_uitaskitem(std::string title, std::string url, std::string json_parms) 
{
	models::M3u8Task m;
	m._title = title;
	m._url = url;
	m._content = json_parms;
	m._folder_name = GetGuid();
	UCTaskItem* pti = _basicForm->AddUCTaskItem(m);
	_map_ucTaskItem.insert(std::make_pair(m._folder_name, pti));
}