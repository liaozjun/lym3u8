#pragma once
#include "run_thread.h"
class MainThread : public nbase::FrameworkThread
{
public:
	MainThread() : nbase::FrameworkThread("MainThread") {}
	virtual ~MainThread() {}

private:
	/**
	* �麯������ʼ�����߳�
	* @return void	�޷���ֵ
	*/
	virtual void Init() override;

	/**
	* �麯�������߳��˳�ʱ����һЩ������
	* @return void	�޷���ֵ
	*/
	virtual void Cleanup() override;
private:
	std::unique_ptr<TaskProcessThread>	task_process_thread_;
	std::unique_ptr<HttpServerThread> http_server_thread_;
};
