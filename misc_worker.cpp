#include "pch.h"
#include "misc_worker.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include "ui_ext_worker.h"
#include "aria2_helper.h"


void MiscWorker::get_all_task() {
	 
	std::list<models::M3u8Task> list;
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{		
		repos::M3u8Repo::QueryAll(db_, list);
	}
	db_.Close();
	Aria2Helper::RunAria2();

	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::get_all_task_callback, UiExtWorker::GetInstance(), list));

	BOOST_FOREACH(models::M3u8Task& t,list) {
		if (t._status == models::M3u8Task::Downloading ||
			t._status == models::M3u8Task::WaitingForDownload) 
		{
			_map_download_queue.insert(std::make_pair(t._folder_name, t));
		}
	}
	nbase::ThreadManager::PostDelayedTask(kThreadMisc, nbase::Bind(&MiscWorker::delay_task_process_download, MiscWorker::GetInstance())
		, nbase::TimeDelta::FromMilliseconds(1000 * 3));
}
void MiscWorker::add_task_item_and_download(models::M3u8Task task)
{
	models::M3u8Task* _task_item_model;
	auto it = _map_download_queue.find(task._folder_name);
	if (it != _map_download_queue.end()) {
		_task_item_model = &(it->second);
	}
	else {
		_map_download_queue.insert(std::make_pair(task._folder_name, task));
		_task_item_model = &(_map_download_queue[task._folder_name]);
	}

	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		if (_task_item_model->_status == models::M3u8Task::Status::Saved) 
		{
			if (_task_item_model->_id == 0) 
			{
				repos::M3u8Repo::Insert(db_, *_task_item_model);
			}
			_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else  if (_task_item_model->_status == models::M3u8Task::Status::WaitingForDownload ||
			_task_item_model->_status == models::M3u8Task::Status::Downloading) 
		{
			_task_item_model->_status = models::M3u8Task::Status::Pause;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else if (_task_item_model->_status == models::M3u8Task::Status::Pause) {
			_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else if (_task_item_model->_status == models::M3u8Task::Status::DownloadComplete) {
			//����ʧ�����ص�ts
			if (_task_item_model->_details_ts.size() == 0) {
				bool res = repos::M3u8Repo::GetTaskDetails(db_, _task_item_model->_id, _task_item_model->_details_ts);
			}
			BOOST_FOREACH(models::M3u8Ts& item, _task_item_model->_details_ts)
			{
				if (item._status == models::M3u8Ts::Status::DownloadComplete &&
					item.errorCode != "0")
				{
					item._status = models::M3u8Ts::Status::Saved;
					item.errorCode = "";
					repos::M3u8Repo::UpdateTaskTsReDownload(db_, item._id, item._status, item.errorCode);
				}
			}
			//����ȴ�����
			_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
	}
	db_.Close();	
	//Sleep(5000);
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::add_task_item_and_download_callback, UiExtWorker::GetInstance(), (*_task_item_model)));
}
void MiscWorker::add_task_item(models::M3u8Task task)
{
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		task._status = models::M3u8Task::Status::Saved;
		repos::M3u8Repo::Insert(db_, task);
	}
	db_.Close();
	//Sleep(3000);
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::add_task_item_callback,UiExtWorker::GetInstance(),task));
}

void MiscWorker::delete_task_item(models::M3u8Task task) {
	//������ض�����ɾ��
	auto it = _map_download_queue.find(task._folder_name);
	if (it != _map_download_queue.end()) {
		_map_download_queue.erase(it);
	}
	LOG(INFO) << "kThreadTaskProcess_Delete";
	//ɾ�����ݿ�
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		//��ȡ��ϸ
		if (task._details_ts.size() == 0) {
			bool res = repos::M3u8Repo::GetTaskDetails(db_, task._id, task._details_ts);
		}
		//ɾ�����ݿ��¼
		repos::M3u8Repo::Delete(db_, task._id);
	}
	db_.Close();
	//ɾ���ļ�
	std::string m3u8path = (boost::format("%1%m3u8\\%2%") % nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory()) % task._folder_name).str();
	std::wstring wm3u8path = nbase::UTF8ToUTF16(m3u8path);
	nbase::DeleteDirectory(wm3u8path);
	//ɾ��aria2 ����
	BOOST_FOREACH(models::M3u8Ts& item, task._details_ts)
	{
		if (!item._aria2_result.empty()) {
			std::string result = Aria2Helper::RequestAria2RemoveDownloadResult(task, item);
		}
	}
	//Sleep(10000);
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::delete_task_item_callback, UiExtWorker::GetInstance(), task));
}

void MiscWorker::delay_task_process_download()
{
	LOG(INFO) << "delay_task_process_download";
	int length = _map_download_queue.size();;
	bool isDownloading = false;

	for (auto it = _map_download_queue.begin(); it != _map_download_queue.end(); it++)
	{
		models::M3u8Task& itsecond = it->second;
		void* pp1 = &(it->second);
		if (itsecond._status == models::M3u8Task::Status::Downloading) {
			isDownloading = true;
			Aria2Helper::_ProcessDownloading(itsecond);
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::update_uitaskitem, UiExtWorker::GetInstance(), itsecond));
			nbase::ThreadManager::PostDelayedTask(kThreadMisc, nbase::Bind(&MiscWorker::delay_task_process_download, MiscWorker::GetInstance())
				, nbase::TimeDelta::FromMilliseconds(3000));
			//������û�д��� ɾ������
			return;
		}
	}
	if (!isDownloading)
	{
		for (auto it = _map_download_queue.begin(); it != _map_download_queue.end(); it++)
		{
			models::M3u8Task& itsecond = it->second;
			void* p = &(it->second);
			void* pp = &itsecond;
			if (itsecond._status == models::M3u8Task::Status::WaitingForDownload) {
				Aria2Helper::ProcessWaitingForDownload(itsecond);
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&UiExtWorker::update_uitaskitem, UiExtWorker::GetInstance(), itsecond));
				nbase::ThreadManager::PostTask(kThreadMisc, nbase::Bind(&MiscWorker::delay_task_process_download, MiscWorker::GetInstance()));
				return;
			}
		}
	}
	nbase::ThreadManager::PostDelayedTask(kThreadMisc, nbase::Bind(&MiscWorker::delay_task_process_download, MiscWorker::GetInstance())
		, nbase::TimeDelta::FromMilliseconds(3000));
}