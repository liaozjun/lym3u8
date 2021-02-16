#pragma once
#include "m3u8_task.h"
#include "http_server_runner.h"
class BasicForm : public ui::WindowImplBase
{
public:
	BasicForm();
	~BasicForm();

	/**
	 * һ�������ӿ��Ǳ���Ҫ��д�Ľӿڣ����������������ӿ�����������
	 * GetSkinFolder		�ӿ�������Ҫ���ƵĴ���Ƥ����Դ·��
	 * GetSkinFile			�ӿ�������Ҫ���ƵĴ��ڵ� xml �����ļ�
	 * GetWindowClassName	�ӿ����ô���Ψһ��������
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;
	/**
	 * �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
	 */
	virtual void InitWindow() override;

	/**
	 * �յ� WM_CLOSE ��Ϣʱ�ú����ᱻ����
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void M3u8LoadComplete(std::string url, std::string json_parms);
	void OnTitleChanged(std::wstring title);
	void TaskListLoading(bool isloading);
	void AddUCTaskItem(models::M3u8Task& mt);
	static const std::wstring kClassName;

private:
	std::wstring _title;
	nim_comp::CefControlBase* cef_control_;
	nim_comp::CefControlBase* cef_control_dev_;
	ui::ListBox*	 list_box_;
	ui::HBox* task_loading_;
private: 
	std::unique_ptr<HttpServerRunner> http_server_runner_;
private:
	void kThreadTaskProcess_GetAllTask( );
	void kThreadTaskProcess_DelayTask_ProcessDownload();
	
};
