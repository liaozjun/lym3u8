#pragma once
#include "tool_kits/db/db_sqlite3.h"
class TaskProcessRunner : public nbase::SupportWeakCallback
{
public:
	void testrun();
	void GetAllTask(void* sender);
	void InserTask(void* sender,void* args);
};

