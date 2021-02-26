#include "pch.h"
#include "uc_task_item.h"
#include <boost/format.hpp>
#include "third_party/curl/include/curl/curl.h"
#include "third_party/jsoncpp/include/json/json.h"
#include <boost/foreach.hpp>
//#include <boost/filesystem.hpp>
UCTaskItem::UCTaskItem()
{
	
}
UCTaskItem::~UCTaskItem()
{
}
static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;

}
void UCTaskItem::InitSubControls(models::M3u8Task& taskItemModel, std::function<void(std::string, models::M3u8Task&)> OnClickBubble)
{
	_OnClickBubble = OnClickBubble;
	_task_item_model = std::make_unique<models::M3u8Task>();
	_task_item_model->_id = taskItemModel._id;
	_task_item_model->_url = taskItemModel._url;// set_request_url(taskItemModel.get_request_url());
	_task_item_model->_title = taskItemModel._title;// set_title(taskItemModel.get_title());
	_task_item_model->_status = taskItemModel._status;
	_task_item_model->_folder_name = taskItemModel._folder_name;
	_task_item_model->_content = taskItemModel._content;// set_context(taskItemModel.get_context());
	_task_item_model->count = taskItemModel.count;
	_task_item_model->count_complete = taskItemModel.count_complete;
	_task_item_model->count_downloading = taskItemModel.count_downloading;
	_task_item_model->count_error = taskItemModel.count_error;

	// 查找 Item 下的控件	 
	label_title_ = dynamic_cast<ui::Label*>(FindSubControl(L"label_title"));
	label_info = dynamic_cast<ui::Label*>(FindSubControl(L"label_info"));
	label_progress = dynamic_cast<ui::Label*>(FindSubControl(L"label_progress"));
	label_title_->SetText(nbase::UTF8ToUTF16(_task_item_model->_title));// get_title()));

	boost::format fmt = boost::format("[%1%]/[%2%]/[%3%]/[%4%]")% _task_item_model->count%_task_item_model->count_complete%_task_item_model->count_error%_task_item_model->count_downloading;
	std::string info = fmt.str();
	label_progress->SetText(nbase::UTF8ToUTF16(info));

	// 绑定删除任务处理函数
	btn_download_save = dynamic_cast<ui::Button*>(FindSubControl(L"btn_download_save"));
	btn_save = dynamic_cast<ui::Button*>(FindSubControl(L"btn_save"));
	btn_play = dynamic_cast<ui::Button*>(FindSubControl(L"btn_play"));
	btn_edit = dynamic_cast<ui::Button*>(FindSubControl(L"btn_edit"));
	btn_del_ = dynamic_cast<ui::Button*>(FindSubControl(L"btn_del"));

	btn_download_save->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_save->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_play->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_edit->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	btn_del_->AttachClick(nbase::Bind(&UCTaskItem::OnClick, this, std::placeholders::_1));
	RefreshCtrls();
}
void UCTaskItem::RefreshCtrls() {
	if (_task_item_model->_status == models::M3u8Task::Status::Saved) {
		if (_task_item_model->_id == 0) {
			//未保存
			label_info->SetText(L"未保存");
			btn_download_save->SetEnabled(true);
			btn_save->SetEnabled(true);
			btn_play->SetEnabled(false);
			btn_edit->SetEnabled(false);
			btn_del_->SetEnabled(false);
		}
		if (_task_item_model->_id != 0) {
			label_info->SetText(L"已保存");
			btn_download_save->SetEnabled(true);
			btn_save->SetEnabled(false);
			btn_play->SetEnabled(false);
			btn_edit->SetEnabled(true);
			btn_del_->SetEnabled(true);
		}
		label_info->SetText(L"下载");
	}else if (_task_item_model->_status == models::M3u8Task::Status::WaitingForDownload) {
		label_info->SetText(L"等待下载");
		btn_download_save->SetEnabled(true);
		btn_download_save->SetText(L"暂停");
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(false);		 

	}else if (_task_item_model->_status == models::M3u8Task::Status::Downloading) {
		label_info->SetText(L"下载中");
		btn_download_save->SetEnabled(true);
		btn_download_save->SetText(L"暂停");
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(false);
	}
	else if (_task_item_model->_status == models::M3u8Task::Status::DownloadComplete) {
		label_info->SetText(L"下载完成");
		btn_download_save->SetEnabled(false);
		btn_download_save->SetText(L"下载");
		if (_task_item_model->count_error != 0) {
			btn_download_save->SetEnabled(true);
			label_info->SetText(L"点击下载失败Ts");
		}
		btn_play->SetEnabled(true);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(true);
	}
	else if (_task_item_model->_status == models::M3u8Task::Status::Pause) {
		label_info->SetText(L"已暂停");
		btn_download_save->SetEnabled(true);
		btn_download_save->SetText(L"下载");
		btn_play->SetEnabled(false);
		btn_edit->SetEnabled(true);
		btn_del_->SetEnabled(true);
	}
	if (_task_item_model->_status != models::M3u8Task::Status::Saved) {
		btn_save->SetVisible(false);
	}
	boost::format fmt = boost::format("[%1%]/[%2%]/[%3%]/[%4%]") % _task_item_model->count%_task_item_model->count_complete%_task_item_model->count_error%_task_item_model->count_downloading;
	std::string info8 = fmt.str();
	std::wstring info = nbase::UTF8ToUTF16(info8);

	label_progress->SetText(info);
}
bool UCTaskItem::OnClick(ui::EventArgs* args)
{
	ui::Button* btn = (ui::Button*)args->pSender;
	std::wstring btnName = btn->GetName();
	if (btnName == L"btn_download_save") {		 
		nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&UCTaskItem::kThreadTaskProcess_InserTaskAndDownload, this));		 
	}
	else if (btnName == L"btn_save") { 
		nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&UCTaskItem::kThreadTaskProcess_InserTask, this));
	}
	else if (btnName == L"btn_play") {
		this->_OnClickBubble(nbase::UTF16ToUTF8(btnName),*(_task_item_model.get()));
	}
	else if (btnName == L"btn_edit") {
		//btn_download_save->SetEnabled(true);
	}
	else if (btnName == L"btn_del") {
		nbase::ThreadManager::PostTask(kThreadTaskProcess, nbase::Bind(&UCTaskItem::kThreadTaskProcess_Delete, this));
	}
	return true;
}
 
 
void UCTaskItem::kThreadTaskProcess_Delete()
{
	LOG(INFO) << "kThreadTaskProcess_Delete";
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		//读取明细
		if (_task_item_model->_details_ts.size() == 0) {
			bool res = repos::M3u8Repo::GetTaskDetails(db_, _task_item_model->_id, _task_item_model->_details_ts);
		}
		//删除数据库记录
		repos::M3u8Repo::Delete(db_, _task_item_model->_id);
	}
	db_.Close();
	//删除文件
	std::string m3u8path = (boost::format("%1%m3u8\\%2%") % nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory())%_task_item_model->_folder_name).str();
	std::wstring wm3u8path = nbase::UTF8ToUTF16(m3u8path);
	//boost::filesystem::remove_all(m3u8path);
//	nbase::DeleteFile(wm3u8path);
	nbase::DeleteDirectory(wm3u8path);
	//删除aria2 任务
	BOOST_FOREACH(models::M3u8Ts& item, _task_item_model->_details_ts)
	{
		if (!item._aria2_result.empty()) {
			std::string result = this->RequestAria2RemoveDownloadResult(item);
		}
	}	
	ui::ListBox* parent = dynamic_cast<ui::ListBox*>(this->GetParent());
	parent->Remove(this);
	//RefreshCtrls();
}

void UCTaskItem::ProcessDownloading() {
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath,pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		//读取明细
		if (_task_item_model->_details_ts.size() == 0) {
			bool res = repos::M3u8Repo::GetTaskDetails(db_, _task_item_model->_id, _task_item_model->_details_ts);
		}
		//是否全部完成
		auto it = std::count_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item) {return item._status == models::M3u8Ts::Status::DownloadComplete; });
		if (it == _task_item_model->_details_ts.size()) 
		{
			this->_task_item_model->_status = models::M3u8Task::Status::DownloadComplete;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else 
		{//未完成 处理
			this->ProcessTsDownload(db_);
		}
		_task_item_model->count = _task_item_model->_details_ts.size();
		//完成
		auto count_complete = std::count_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item) {
			return item._status == models::M3u8Ts::Status::DownloadComplete;
		});
		_task_item_model->count_complete = count_complete;
		//错误
		auto count_error = std::count_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item) {
			return item._status == models::M3u8Ts::Status::DownloadComplete && item.errorCode != "0" && !item.errorCode.empty();
			});
		_task_item_model->count_error = count_error;
		//下载
		auto count_downloading = std::count_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item) {
			return item._status == models::M3u8Ts::Status::Downloading;
			});
		_task_item_model->count_downloading = count_downloading;
	}
	db_.Close();	
	RefreshCtrls();
}

void UCTaskItem::ProcessWaitingForDownload() {
	_task_item_model->_details_ts.clear();
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(),pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{

		if (_task_item_model->_details_ts.size() == 0)
		{
			repos::M3u8Repo::GetTaskDetails(db_, _task_item_model->_id, _task_item_model->_details_ts);
		}

		this->_task_item_model->_status = models::M3u8Task::Status::Downloading;
		repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);

	}
	db_.Close();
	
}
std::string GetRequestCommand(std::string action,models::M3u8Task& task,models::M3u8Ts& ts) 
{
	Json::FastWriter writer;
	Json::Value value;
	Json::Value array;
	if (action == "aria2.addUri") {
		/*
		{"id":"qwer1","jsonrpc":"2.0","method":"aria2.addUri"
,"params":[["https://product-downloads.atlassian.com/software/sourcetree/windows/ga/SourceTreeSetup-3.4.2.exe"]]}
*/		
		//url
		Json::Value array1;
		array1.append(ts._url);
		array.append(array1);
		//dir
		std::string theme_dir = nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory());
		Json::Value parms;
		parms["dir"] = theme_dir + "m3u8\\" + task._folder_name;
		array.append(parms);

		value["jsonrpc"] = "2.0";
		value["id"] = nbase::Int64ToString(ts._id);// "qwer";
		value["method"] = "aria2.addUri";
		value["params"] = array;
	}
	else if (action == "aria2.tellStatus") {
		/*{"jsonrpc":"2.0", "id" : "qwer",
			"method" : "aria2.tellStatus",
			"params" : ["d1d2b5c3acafab0c", ["gid", "totalLength", "completedLength", "errorCode", "status", "errorMessage"]]}*/
		value["jsonrpc"] = "2.0";
		value["id"] = nbase::Int64ToString(ts._id);// "qwer";
		value["method"] = "aria2.tellStatus";		
		Json::Value array1;
		array1.append("gid");
		array1.append("errorCode");
		array1.append("status");
		array1.append("errorMessage");

		array.append(ts._aria2_result);
		array.append(array1);
		value["params"] = array;
	}
	else if (action == "aria2.removeDownloadResult") {
		//{"id":"qwer1","jsonrpc":"2.0","method":"aria2.removeDownloadResult","params":["13e706faf06571b2"]}
		value["jsonrpc"] = "2.0";
		value["id"] = nbase::Int64ToString(ts._id);// "qwer";
		value["method"] = "aria2.removeDownloadResult";
		array.append(ts._aria2_result);	 
		value["params"] = array;
	}
	std::string json_file = writer.write(value);
	return json_file;
}
std::string UCTaskItem::RequestAria2AddUri(models::M3u8Ts& ts) {
	std::string json_file = GetRequestCommand("aria2.addUri", *(_task_item_model.get()), ts);
	return this->RequestAria2(json_file);
}
std::string UCTaskItem::RequestAria2TellStatus(models::M3u8Ts& it)
{
	//请求状态
	std::string json_file = GetRequestCommand("aria2.tellStatus", *(_task_item_model.get()), it);
	return this->RequestAria2(json_file);
}
std::string UCTaskItem::RequestAria2RemoveDownloadResult(models::M3u8Ts& it) {
	std::string json_file = GetRequestCommand("aria2.removeDownloadResult", *(_task_item_model.get()), it);
	return this->RequestAria2(json_file);
}
bool UCTaskItem::ProcessTsDownload(ndb::SQLiteDB& db)
{
	//find ts 正在下载的 是否完成（包括出错停止） 完成更新对象和数据库状态。
	/*auto it = std::find_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item) { return item._status == models::M3u8Ts::Status::Downloading; });
	if (it != std::end(_task_item_model->_details_ts)) {
		this->RequestAria2Status(it->_aria2_result);
	}*/
	BOOST_FOREACH(models::M3u8Ts& item, _task_item_model->_details_ts)
	{
		if (item._status == models::M3u8Ts::Status::Downloading) 
		{
			/*Json::FastWriter writer;
			std::string = writer.write(item);*/
			std::string jsonRes = this->RequestAria2TellStatus(item);
			LOG(INFO) << "RequestAria2TellStatus:" << item._url << jsonRes;
			/*error complete
			"id": "qwer",
	"jsonrpc": "2.0",
	"result": {
		"completedLength": "0",
		"errorCode": "1",
		"errorMessage": "Network problem has occurred. cause:No connection could be made because the target machine actively refused it.\r\n",
		"gid": "ff271475a6a173df",
		"status": "error",
		"totalLength": "0"
	}
	removeDownloadResult
	如果错误删除aria2 的 task
			*/
			if (!jsonRes.empty()) {
				Json::Value root;
				Json::Reader jr;
				jr.parse(jsonRes, root);
				std::string result_status = root["result"]["status"].asString();
				if (result_status == "error" || result_status == "complete") {
					item.errorCode = root["result"]["errorCode"].asString();
					item._status = models::M3u8Ts::Status::DownloadComplete;
					item.errorMessage = root["result"]["errorMessage"].asString();
					repos::M3u8Repo::UpdateTaskTsStatus(db, item._id, item._status, item.errorCode, item.errorMessage);
					if (item.errorCode != "0") {
						this->RequestAria2RemoveDownloadResult(item);
					}
				}else
				{//{"id":"26704","jsonrpc":"2.0","error":{"code":1,"message":"GID 13e706faf06571b2 is not found"}}
					item.errorCode = root["error"]["code"].asString();					
					if (!item.errorCode.empty()) {
						item._status = models::M3u8Ts::Status::DownloadComplete;
						item.errorMessage = root["error"]["message"].asString();
						repos::M3u8Repo::UpdateTaskTsStatus(db, item._id, item._status, item.errorCode, item.errorMessage);
					}
					if (item.errorCode != "0") {
						this->RequestAria2RemoveDownloadResult(item);
					}
				}
			}
			else {

			}
		}
	}
	
	//find 如果正在下载 不足 10个 在发起10 个或余下的
	const int request_tick = 10;
	auto count_downloading= std::count_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item)
		{ return item._status == models::M3u8Ts::Status::Downloading; });
	LOG(INFO) << "Current Downloading:" << count_downloading;
	if (count_downloading < request_tick) {
		//get 10 saved
		int saved_tick = 0;
		BOOST_FOREACH(models::M3u8Ts& item, _task_item_model->_details_ts)
		{
			if (item._status == models::M3u8Ts::Status::Saved)
			{

				std::string jsonRes = this->RequestAria2AddUri(item);
				LOG(INFO) << "RequestAria2AddUri:" << item._url << jsonRes;
				//json parse strResponse result 
				/*
				{
		"id": "qwer1",
		"jsonrpc": "2.0",
		"result": "2bc4d00f31295d4a"
		}
					*/
				if (!jsonRes.empty()) {
					Json::Value root;
					Json::Reader jr;
					jr.parse(jsonRes, root);
					std::string aria2_result = root["result"].asString();
					if (!aria2_result.empty()) {
						item._status = models::M3u8Ts::Status::Downloading;
						item._aria2_result = aria2_result;
						//update ts
						repos::M3u8Repo::UpdateTaskTsStatus(db, item._id, item._aria2_result, item._status);
					}
				}
				else {
					LOG(INFO) << "RequestAria2AddUri else jsonRes :" << jsonRes;
				}
				saved_tick++;
				if (saved_tick == request_tick) {
					break;
				}
			}
		}
	}
	return true;
}
std::string UCTaskItem::RequestAria2(std::string& cmd) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return false;// CURLE_FAILED_INIT;
	}
	/*if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}*/
	std::string strResponse;
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:6800/jsonrpc");
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cmd.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, cmd.size());//设置上传json串长度,这个设置可以忽略
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_slist_free_all(headers); /* free the list again */
	curl_easy_cleanup(curl);
	//CURLE_OK
	return strResponse;
}

void UCTaskItem::kThreadTaskProcess_InserTask() {
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{//int res = db_.Query("CREATE TABLE [M3u8Task] ([id] INTEGER  NOT NULL PRIMARY KEY,[status] INTEGER DEFAULT '0' NULL,[title] VARCHAR(255)  NULL,[request_url] VARCHAR(1024)  NULL,[folder_name] vARCHAR(255)  NULL,[aria2_request_status] INTEGER DEFAULT '0' NULL,[aria2_download_status] INTEGER DEFAULT '0' NULL,[context] TEXT  NULL)");
	//	//int rowcount = models::M3u8Task::GetCountBy(db_, _task_item_model->get_title());
		_task_item_model->_status = models::M3u8Task::Status::Saved;
		repos::M3u8Repo::Insert(db_, *_task_item_model);
		RefreshCtrls();
	}
	db_.Close();
}

void UCTaskItem::kThreadTaskProcess_InserTaskAndDownload()
{
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		if (_task_item_model->_status == models::M3u8Task::Status::Saved) 
		{
			if (_task_item_model->_id == 0) 
			{
				repos::M3u8Repo::Insert(db_, *_task_item_model);
			}
			_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else  if (_task_item_model->_status == models::M3u8Task::Status::WaitingForDownload ||
			_task_item_model->_status == models::M3u8Task::Status::Downloading) 
		{
			_task_item_model->_status = models::M3u8Task::Status::Pause;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else if (_task_item_model->_status == models::M3u8Task::Status::Pause) {
			_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
		else if (_task_item_model->_status == models::M3u8Task::Status::DownloadComplete) {
			//重置失败下载的ts
			if (_task_item_model->_details_ts.size() == 0) {
				bool res = repos::M3u8Repo::GetTaskDetails(db_, _task_item_model->_id, _task_item_model->_details_ts);
			}
			BOOST_FOREACH(models::M3u8Ts& item, _task_item_model->_details_ts)
			{
				if (item._status == models::M3u8Ts::Status::DownloadComplete &&
					item.errorCode != "0")
				{
					item._status = models::M3u8Ts::Status::Saved;
					item.errorCode = "";
					repos::M3u8Repo::UpdateTaskTsReDownload(db_, item._id, item._status, item.errorCode);
				}
			}
			//进入等待下载
			_task_item_model->_status = models::M3u8Task::Status::WaitingForDownload;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model->_id, _task_item_model->_status);
		}
	}
	db_.Close();	
	RefreshCtrls();
}
//int res = db_.Query("CREATE TABLE [M3u8Task] ([id] INTEGER  NOT NULL PRIMARY KEY,[status] INTEGER DEFAULT '0' NULL,[title] VARCHAR(255)  NULL,[request_url] VARCHAR(1024)  NULL,[folder_name] vARCHAR(255)  NULL,[aria2_request_status] INTEGER DEFAULT '0' NULL,[aria2_download_status] INTEGER DEFAULT '0' NULL,[context] TEXT  NULL)");
		//	//int rowcount = models::M3u8Task::GetCountBy(db_, _task_item_model->get_title());