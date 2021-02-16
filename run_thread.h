#pragma once

//kThreadTaskProcess
class TaskProcessThread : public nbase::FrameworkThread
{
public:
	TaskProcessThread(enum ThreadId thread_id, const char *name)
		: FrameworkThread(name)
		, thread_id_(thread_id) {}

	~TaskProcessThread(void) {}

private:
	/**
	* �麯������ʼ���߳�
	* @return void	�޷���ֵ
	*/
	virtual void Init() override;

	/**
	* �麯�����߳��˳�ʱ����һЩ������
	* @return void	�޷���ֵ
	*/
	virtual void Cleanup() override;

private:
	enum ThreadId thread_id_;

};

//kThreadHttpServer,
//mogoose http server
class HttpServerThread : public nbase::FrameworkThread {
public:
	HttpServerThread(enum ThreadId thread_id, const char *name)
		: FrameworkThread(name)
		, thread_id_(thread_id) {}

	~HttpServerThread(void) {}

private:
	/**
	* �麯������ʼ���߳�
	* @return void	�޷���ֵ
	*/
	virtual void Init() override;

	/**
	* �麯�����߳��˳�ʱ����һЩ������
	* @return void	�޷���ֵ
	*/
	virtual void Cleanup() override;

private:
	enum ThreadId thread_id_;
};