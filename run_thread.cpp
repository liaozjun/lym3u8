#include "pch.h"
#include "run_thread.h"

void TaskProcessThread::Init()
{
	nbase::ThreadManager::RegisterThread(thread_id_);
}

void TaskProcessThread::Cleanup()
{
	nbase::ThreadManager::UnregisterThread();
}
 
void HttpServerThread::Init()
{
	nbase::ThreadManager::RegisterThread(thread_id_);
}

void HttpServerThread::Cleanup()
{
	nbase::ThreadManager::UnregisterThread();
}