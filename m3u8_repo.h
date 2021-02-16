#pragma once
#include "tool_kits/db/db_sqlite3.h"
#include "m3u8_task.h"
namespace repos {
	class M3u8Repo
	{
	public:
		static int GetCountBy(ndb::SQLiteDB& db, std::string title);
		static int Insert(ndb::SQLiteDB& db, models::M3u8Task& task);
		static bool QueryAll(ndb::SQLiteDB& db, std::list<models::M3u8Task>& list);
		static int Delete(ndb::SQLiteDB& db, int64 m3u8_task_id);
		static int UpdateTaskStatus(ndb::SQLiteDB& db, int64 m3u8_task_id, models::M3u8Task::Status status);
		static bool GetTaskDetails(ndb::SQLiteDB& db, int64 m3u8_task_id, std::list<models::M3u8Ts>& list);
	};
}
