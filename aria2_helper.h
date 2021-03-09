#pragma once
#include "m3u8_task.h"
#include "m3u8_repo.h"
class Aria2Helper
{
public:
	static void RunAria2();
	static void Aria2Conf();

	static std::string GetRequestCommand(std::string action, models::M3u8Task& task, models::M3u8Ts& ts);
	static void _ProcessDownloading(models::M3u8Task& _task_item_model);
	static bool _ProcessTsDownload(ndb::SQLiteDB& db, models::M3u8Task& _task_item_model);
	static std::string RequestAria2(std::string& cmd);
	static std::string RequestAria2TellStatus(models::M3u8Task& ti, models::M3u8Ts& ts);
	static std::string RequestAria2AddUri(models::M3u8Task& ti, models::M3u8Ts& ts);
	static std::string RequestAria2RemoveDownloadResult(models::M3u8Task& ti, models::M3u8Ts& ts);
	static void ProcessWaitingForDownload(models::M3u8Task& _task_item_model);
};

