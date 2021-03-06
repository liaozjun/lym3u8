#pragma once
#include "m3u8_task.h"
#include "http_server_runner.h"
#include "task_manager_worker.h"
#include "uc_task_item.h"
class BasicForm : public ui::WindowImplBase
{
public:
	BasicForm();
	~BasicForm();
	static const std::wstring kClassName;
	/**
	 * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;
	/**
	 * 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	 */
	virtual void InitWindow() override;

	/**
	 * 收到 WM_CLOSE 消息时该函数会被调用
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void M3u8LoadComplete(std::string url, std::string json_parms);
	void OnTitleChanged(std::wstring title);
	void TaskListLoading(bool isloading);
	UCTaskItem* AddUCTaskItem(models::M3u8Task& mt);
	
 
	void OnClickBubble(std::string action, models::M3u8Task& task);
	void OnLoadEnd(int httpStatusCode);
	bool OnClick(ui::EventArgs* args);
	bool EditUrlReturn(ui::EventArgs* args);

	 
private:
	std::wstring _title;
	nim_comp::CefControlBase* cef_control_;
	nim_comp::CefControlBase* cef_control_dev_;
	ui::ListBox*	 list_box_;
	std::string cur_url;
	ui::HBox* task_loading_;
	ui::RichEdit* edit_url;
	ui::Button* btn_navigate;
	ui::Button* btn_refresh;
	ui::Button* btn_home;
private: 
	std::unique_ptr<HttpServerRunner> http_server_runner_;
	//std::unique_ptr<TaskManagerWorker> task_manager_worker;
 
	
};

