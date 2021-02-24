#include "pch.h"
#include "m3u8_repo.h"
#include "boost/format.hpp"
#include <objbase.h>
namespace repos {
	
	///////////////////////////////////////////////////////////////
	void M3u8Repo::GetDbInfo(std::string& dbPath, std::string& pwd) {
		std::string curpath = nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory());
		dbPath = curpath + "lygg";
		pwd = "";
	}
	int M3u8Repo::Delete(ndb::SQLiteDB& db, int64 m3u8_task_id)
	{
		boost::format fmt = boost::format("delete * from m3u8_task where id = %d") % m3u8_task_id;
		boost::format fmtd = boost::format("delete * from m3u8_ts where m3u8_task_id = %d") % m3u8_task_id;
		std::string sqlt = fmt.str();
		int ret = db.Query(sqlt.data());
		std::string sqlts = fmtd.str();
		ret = db.Query(sqlts.data());
		return true;
	}
	int M3u8Repo::GetCountBy(ndb::SQLiteDB& db, std::string title) {
		ndb::SQLiteResultTable dt;
		std::string queryExist = "select count(*) from M3u8Task where title = '" + title + "'";
		int res1 = db.Query(queryExist.data(), dt);
		int rowcount = 0;
		if (dt.GetRowCount() > 0) {
			std::string rowcounttmp = dt.GetValue(0, 0);
			nbase::StringToInt(rowcounttmp, &rowcount);
		}
		dt.Free();
		return rowcount;
	}
	bool M3u8Repo::QueryAll(ndb::SQLiteDB& db, std::list<models::M3u8Task>& list) 
	{
		boost::format fmtd = boost::format("select t.id,t.title,t.url,t.status,t.folder_name,t.content,t.create_time,t.end_time,\n"
			"(select count(*) from m3u8_ts ts where ts.m3u8_task_id = t.id) as count,\n"
			"(select count(*) from m3u8_ts ts where ts.m3u8_task_id = t.id and ts.status = %1%) as count_downloading,\n"
			"(select count(*) from m3u8_ts ts where ts.m3u8_task_id = t.id and ts.status = %2%) as count_complete,\n"
			"(select count(*) from m3u8_ts ts where ts.m3u8_task_id = t.id and ts.status = %2% and ts.errorCode != 0 and ts.errorCode is not null) as count_error\n"
			"from m3u8_task t") % models::M3u8Ts::Downloading%models::M3u8Ts::DownloadComplete;
		std::string queryExist = fmtd.str();
		ndb::SQLiteStatement stat;
		int ret = db.Query(stat, queryExist.data());
		if (!stat.IsValid()) {
			stat.Finalize();
			return false;
		}
		while (stat.NextRow() != SQLITE_DONE) {
			try {
				models::M3u8Task t;
				int64 id = stat.GetInt64Field(0);
				std::string title = stat.GetTextField(1);
				std::string url = stat.GetTextField(2);
				int64 status = stat.GetInt64Field(3);
				std::string folder_name = stat.GetTextField(4);
				std::string 	content = stat.GetTextField(5);				 
				const char* ct = stat.GetTextField(6);
				if (ct != NULL) { 
					nbase::StringToInt64(ct, &(t.create_time));
				}
				const char* et = stat.GetTextField(7); 
				if (et != NULL) {
					nbase::StringToInt64(et, &(t.end_time));
				}
				t.LoadDbInit(id, title, url, (models::M3u8Task::Status)status, folder_name, content,t.create_time,t.end_time);
				t.count = stat.GetIntField(8);
				t.count_downloading = stat.GetIntField(9);
				t.count_complete = stat.GetIntField(10);
				t.count_error = stat.GetIntField(11);
				list.push_back(t);
			}
			catch (...) {
				LOG(INFO) << "QueryAll";
			}
		}
		stat.Finalize();
		return true;
	}
	int M3u8Repo::Insert(ndb::SQLiteDB& db, models::M3u8Task& task) {
		char buffer[64] = { 0 };
		GUID guid;
		if (CoCreateGuid(&guid))
		{
			//fprintf(stderr, "create guid error\n");
			return -1;
		}
		_snprintf_s(buffer, sizeof(buffer),
			"%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
		task._folder_name = buffer;
		nbase::Time tm = nbase::Time::Now();		
		int64_t iv = tm.ToInternalValue();
		task.create_time = iv;
		std::string ct = nbase::Int64ToString(iv);
		std::string insertSql = "insert into m3u8_task(title,\n"
			"url ,\n"
			"status,\n"
			"folder_name, \n"
			"content, \n"
			"create_time) values(\n'"
			+ task._title + "','"
			+ task._url + "',0,'"
			+ task._folder_name + "','"
			+ task._content+"','"
			+ ct +
			"')";
		const char* aa = insertSql.data();
		int resi = db.Query(insertSql.data());
		std::string lastid = "select last_insert_rowid() as id;";
		ndb::SQLiteResultTable table;
		resi = db.Query(lastid.data(), table);
		std::string id = table.GetValue(0, 0);
		table.Free();
		nbase::StringToInt64(id, &(task._id));

		std::list<std::string> urls;
		std::list<std::string> allm3u8;
		task.ProcessContext(urls, allm3u8);

		std::string indexm3u8all;
		auto joinall = [&indexm3u8all](std::string str) {
			indexm3u8all.append(str);
			indexm3u8all.append("\n");
		};
		std::for_each(allm3u8.begin(), allm3u8.end(), joinall);
		//
		std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
		std::wstring path = theme_dir+ L"m3u8\\" + nbase::UTF8ToUTF16(task._folder_name) + L"\\";
		nbase::CreateDirectory(path);
		int f = nbase::WriteFile(path + L"index.m3u8", indexm3u8all);

		auto insert_detail = [&task, &db,&ct](std::string str) {
			boost::format fmt = boost::format("insert into m3u8_ts(m3u8_task_id,aria2_result,status,url,create_time) values (%1%,'',0,'%2%','%3%')") % task._id % str % ct;
			std::string sqlts = fmt.str();
			int ret = db.Query(sqlts.data());
		};
		std::for_each(urls.begin(), urls.end(), insert_detail);

		return resi;
	}
	int M3u8Repo::UpdateTaskStatus(ndb::SQLiteDB& db,int64 m3u8_task_id, models::M3u8Task::Status status)
	{
		boost::format fmt = boost::format(
			"update m3u8_task set status=%1% where id=%2%") % status % m3u8_task_id;
		std::string sqlts = fmt.str();
		int ret = db.Query(sqlts.data());

		return ret;
	}
	int M3u8Repo::UpdateTaskTsStatus(ndb::SQLiteDB& db, int64 ts_id, std::string aria2_result, models::M3u8Ts::Status status)
	{
		boost::format fmt = boost::format(
			"update m3u8_ts set status=%1%,aria2_result='%2%'  where id = %3%") % status % aria2_result % ts_id;
		std::string sqlts = fmt.str();
		int ret = db.Query(sqlts.data());
		return 0;
	}
	int M3u8Repo::UpdateTaskTsStatus(ndb::SQLiteDB& db, int64 ts_id, models::M3u8Ts::Status status, std::string errorCode,std::string errorMessage)
	{
		boost::format fmt = boost::format(
			"update m3u8_ts set status=%1%,errorCode='%2%',errorMessage='%3%' where id = %4%") % status % errorCode %errorMessage%ts_id;
		std::string sqlts = fmt.str();
		int ret = db.Query(sqlts.data());
		return 0;
	}
	int M3u8Repo::UpdateTaskTsReDownload(ndb::SQLiteDB& db, int64 ts_id, models::M3u8Ts::Status status, std::string errCode)
	{
		boost::format fmt = boost::format(
			"update m3u8_ts set status=%1%,errorCode='%2%',errorMessage='%3%' where id = %4%") % status % errCode %""%ts_id;
		std::string sqlts = fmt.str();
		int ret = db.Query(sqlts.data());
		return 0;
	}
	bool M3u8Repo::GetTaskDetails(ndb::SQLiteDB& db, int64 m3u8_task_id, std::list<models::M3u8Ts>& list)
	{
		boost::format fmt = boost::format(
			"select id,m3u8_task_id,aria2_result,status,url,create_time,errorCode from m3u8_ts where m3u8_task_id = %1%") % m3u8_task_id;
		std::string sqlts = fmt.str();

		ndb::SQLiteStatement stat;
		int ret = db.Query(stat, sqlts.data());
		if (!stat.IsValid()) {
			stat.Finalize();
			return false;
		}
		while (stat.NextRow() != SQLITE_DONE) {
			try {
				models::M3u8Ts ts;
				 
				ts._id = stat.GetInt64Field(0);
				nbase::StringToInt64(stat.GetTextField(1), &(ts._m3u8_task_id));
				ts._aria2_result = stat.GetTextField(2);
				int64_t status = stat.GetInt64Field(3);
				ts._status = (models::M3u8Ts::Status)status;
				ts._url = stat.GetTextField(4);
				const char* ct = stat.GetTextField(5);				  
				if (ct != NULL) {
					nbase::StringToInt64(ct, &(ts._create_time));
				}
				const char* errCode = stat.GetTextField(6);
				if (errCode != NULL) {
					ts.errorCode = errCode;
				}
				list.push_back(ts);
			}
			catch (...) {
				LOG(INFO) << "GetTaskDetail";
			}
		}
		stat.Finalize();
		return true;
	}
	bool M3u8Repo::GetM3u8Task(ndb::SQLiteDB& db, std::string url, models::M3u8Task& task)
	{ 
		boost::format fmt = boost::format(
			"select id, title, url, status, folder_name, content, create_time, end_time from m3u8_task where url = '%1%'") % url;
		std::string sql = fmt.str();
		ndb::SQLiteStatement stat;
		int ret = db.Query(stat, sql.data());
		if (!stat.IsValid()) {
			stat.Finalize();
			return false;
		}
		while (stat.NextRow() != SQLITE_DONE) {
			try {
				int64 id = stat.GetInt64Field(0);
				std::string title = stat.GetTextField(1);
				std::string url = stat.GetTextField(2);
				int64 status = stat.GetInt64Field(3);
				std::string folder_name = stat.GetTextField(4);
				std::string 	content = stat.GetTextField(5);
				const char* ct = stat.GetTextField(6);
				if (ct != NULL) {
					nbase::StringToInt64(ct, &(task.create_time));
				}
				const char* et = stat.GetTextField(7);
				if (et != NULL) {
					nbase::StringToInt64(et, &(task.end_time));
				}
				task.LoadDbInit(id, title, url, (models::M3u8Task::Status)status, folder_name, content, task.create_time, task.end_time);
			}
			catch (...) {
				LOG(INFO) << "QueryAll";
			}
		}
		stat.Finalize();
		return true;
	}
	//////////////////////////////////////////////////////////////
}