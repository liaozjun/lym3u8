#include "pch.h"
#include "task_process_runner.h"
#include "m3u8_task.h"
#include "m3u8_repo.h"
#include "basic_form.h"

void TaskProcessRunner::testrun() 
{

}

void TaskProcessRunner::GetAllTask(void* sender) {
	BasicForm* bf = (BasicForm*)sender;
	ndb::SQLiteDB db_;
	bool result = db_.Open("./lygg", "", ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		std::list<models::M3u8Task> list;
		repos::M3u8Repo::QueryAll(db_, list);
		for (auto it = list.begin(); it != list.end(); it++)// auto mt : list)
		{
			bf->AddUCTaskItem(*it);
		}
	}
	db_.Close();

	bf->TaskListLoading(false);
}

void TaskProcessRunner::InserTask(void* sender,void* args) {
	BasicForm* bf = (BasicForm*)sender;
	models::M3u8Task
	ndb::SQLiteDB db_;
	bool result = db_.Open("./lygg", "", ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{//int res = db_.Query("CREATE TABLE [M3u8Task] ([id] INTEGER  NOT NULL PRIMARY KEY,[status] INTEGER DEFAULT '0' NULL,[title] VARCHAR(255)  NULL,[request_url] VARCHAR(1024)  NULL,[folder_name] vARCHAR(255)  NULL,[aria2_request_status] INTEGER DEFAULT '0' NULL,[aria2_download_status] INTEGER DEFAULT '0' NULL,[context] TEXT  NULL)");
	//	//int rowcount = models::M3u8Task::GetCountBy(db_, _task_item_model->get_title());
		repos::M3u8Repo::Insert(db_, *args);
		RefreshCtrls();
	}
	db_.Close();
}