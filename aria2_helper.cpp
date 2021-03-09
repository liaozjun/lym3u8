#include "pch.h"
#include "aria2_helper.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include "third_party/curl/include/curl/curl.h"
#include "third_party/jsoncpp/include/json/json.h"

u_short rpc_listen_port = 6800;
void Aria2Helper::Aria2Conf() {
	u_short port = 6800;
	for (int i = 0; i < 1000; i++) {
		port = port + i;
		if (TestBindPort(port)) {
			break;
		}
	}
	rpc_listen_port = port;
	std::string path = nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory());
	boost::format arai2conf = boost::format(
#ifdef _DEBUG
		"log=%1%static\\log\\aria2.log\n"
#else 
		"quiet=true\n"
#endif
		"daemon=true\n"
		"input-file=%1%static\\aria2.session\n"
		"save-session=%1%static\\aria2.session\n"
		"save-session-interval=30\n"
		"continue=true\n"
		"file-allocation=prealloc\n"
		"user-agent=Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36\n"
		"disable-ipv6=true\n"
		"always-resume=true\n"
		"check-integrity=true\n"
		"max-concurrent-downloads=10\n"
		"max-connection-per-server=5\n"
		"min-split-size=10M\n"
		"split=5\n"
		"enable-rpc=true\n"
		"rpc-allow-origin-all=true\n"
		"rpc-listen-port=%2%\n") % path%rpc_listen_port;
	std::string alltext = arai2conf.str();
	int f = nbase::WriteFile(nbase::win32::GetCurrentModuleDirectory() + L"static\\aria2.conf", alltext);
}

void Aria2Helper::RunAria2() {
	Aria2Helper::Aria2Conf();
	std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
	DWORD processId = GetCurrentProcessId();//当前进程id
	//--stop-with-process=
	std::wstring pid = nbase::Uint64ToString16(processId);
	std::wstring cmdline = nbase::StringPrintf(L"%sstatic\\aria2c.exe --conf-path=%sstatic\\aria2.conf --check-certificate=false --stop-with-process=%s",
		theme_dir.data(), theme_dir.data(), pid.data());
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	//隐藏掉可能出现的cmd命令窗口
	si.dwFlags = STARTF_USESHOWWINDOW;
#ifdef _DEBUG
	si.wShowWindow = SW_SHOW;//SW_HIDE
#else
	si.wShowWindow = SW_HIDE;
#endif // _DEBUG	
	ZeroMemory(&pi, sizeof(pi));
	BOOL bRet = ::CreateProcess(
		NULL,//启动程序路径名 
		(LPWSTR)(cmdline.c_str()), //参数（当exeName为NULL时，可将命令放入参数前） 
		NULL, //使用默认进程安全属性 
		NULL, //使用默认线程安全属性 
		FALSE,//句柄不继承 
		0, //使用正常优先级 
		NULL, //使用父进程的环境变量 
		NULL, //指定工作目录 
		&si, //子进程主窗口如何显示 
		&pi); //用于存放新进程的返回信息 

		//NULL, (LPWSTR)(cmdline.c_str()), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	DWORD ecode = 0;
	if (bRet == 0)
	{
		ecode = GetLastError();
	}
}

void Aria2Helper::ProcessWaitingForDownload(models::M3u8Task& _task_item_model) {
	_task_item_model._details_ts.clear();
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		if (_task_item_model._details_ts.size() == 0)
		{
			repos::M3u8Repo::GetTaskDetails(db_, _task_item_model._id, _task_item_model._details_ts);
		}
		_task_item_model._status = models::M3u8Task::Status::Downloading;
		repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model._id, _task_item_model._status);
	}
	db_.Close();
}
void Aria2Helper::_ProcessDownloading(models::M3u8Task& _task_item_model) {
	std::string dbpath;
	std::string pwd;
	repos::M3u8Repo::GetDbInfo(dbpath, pwd);
	ndb::SQLiteDB db_;
	bool result = db_.Open(dbpath.data(), pwd, ndb::SQLiteDB::modeReadWrite | ndb::SQLiteDB::modeCreate | ndb::SQLiteDB::modeSerialized);
	if (result)
	{
		//读取明细
		if (_task_item_model._details_ts.size() == 0) {
			bool res = repos::M3u8Repo::GetTaskDetails(db_, _task_item_model._id, _task_item_model._details_ts);
		}
		//是否全部完成
		auto it = std::count_if(std::begin(_task_item_model._details_ts), std::end(_task_item_model._details_ts), [](models::M3u8Ts& item) {return item._status == models::M3u8Ts::Status::DownloadComplete; });
		if (it == _task_item_model._details_ts.size())
		{
			_task_item_model._status = models::M3u8Task::Status::DownloadComplete;
			repos::M3u8Repo::UpdateTaskStatus(db_, _task_item_model._id, _task_item_model._status);
		}
		else
		{//未完成 处理
			Aria2Helper::_ProcessTsDownload(db_, _task_item_model);
		}
		_task_item_model.count = _task_item_model._details_ts.size();
		//完成
		auto count_complete = std::count_if(std::begin(_task_item_model._details_ts), std::end(_task_item_model._details_ts), [](models::M3u8Ts& item) {
			return item._status == models::M3u8Ts::Status::DownloadComplete;
			});
		_task_item_model.count_complete = count_complete;
		//错误
		auto count_error = std::count_if(std::begin(_task_item_model._details_ts), std::end(_task_item_model._details_ts), [](models::M3u8Ts& item) {
			return item._status == models::M3u8Ts::Status::DownloadComplete && item.errorCode != "0" && !item.errorCode.empty();
			});
		_task_item_model.count_error = count_error;
		//下载
		auto count_downloading = std::count_if(std::begin(_task_item_model._details_ts), std::end(_task_item_model._details_ts), [](models::M3u8Ts& item) {
			return item._status == models::M3u8Ts::Status::Downloading;
			});
		_task_item_model.count_downloading = count_downloading;
	}
	db_.Close();
	//RefreshCtrls();
}
bool Aria2Helper::_ProcessTsDownload(ndb::SQLiteDB& db, models::M3u8Task& _task_item_model)
{
	//find ts 正在下载的 是否完成（包括出错停止） 完成更新对象和数据库状态。
	/*auto it = std::find_if(std::begin(_task_item_model->_details_ts), std::end(_task_item_model->_details_ts), [](models::M3u8Ts& item) { return item._status == models::M3u8Ts::Status::Downloading; });
	if (it != std::end(_task_item_model->_details_ts)) {
		this->RequestAria2Status(it->_aria2_result);
	}*/
	BOOST_FOREACH(models::M3u8Ts& item, _task_item_model._details_ts)
	{
		if (item._status == models::M3u8Ts::Status::Downloading)
		{
			/*Json::FastWriter writer;
			std::string = writer.write(item);*/
			std::string jsonRes = Aria2Helper::RequestAria2TellStatus(_task_item_model, item);
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
						Aria2Helper::RequestAria2RemoveDownloadResult(_task_item_model, item);
					}
				}else if (result_status == "active") 
				{
					LOG(INFO) << "active";
				}
				else
				{//{"id":"26704","jsonrpc":"2.0","error":{"code":1,"message":"GID 13e706faf06571b2 is not found"}}
					item.errorCode = root["error"]["code"].asString();
					if (!item.errorCode.empty()) {
						item._status = models::M3u8Ts::Status::DownloadComplete;
						item.errorMessage = root["error"]["message"].asString();
						repos::M3u8Repo::UpdateTaskTsStatus(db, item._id, item._status, item.errorCode, item.errorMessage);
					}
					if (item.errorCode != "0") {
						Aria2Helper::RequestAria2RemoveDownloadResult(_task_item_model, item);
					}
				}
			}
			else {

			}
		}
	}

	//find 如果正在下载 不足 10个 在发起10 个或余下的
	const int request_tick = 5;
	auto count_downloading = std::count_if(std::begin(_task_item_model._details_ts), std::end(_task_item_model._details_ts), [](models::M3u8Ts& item)
		{ return item._status == models::M3u8Ts::Status::Downloading; });
	LOG(INFO) << "Current Downloading:" << count_downloading;
	if (count_downloading < request_tick) {
		//get 10 saved
		int saved_tick = 0;
		BOOST_FOREACH(models::M3u8Ts& item, _task_item_model._details_ts)
		{
			if (item._status == models::M3u8Ts::Status::Saved)
			{

				std::string jsonRes = Aria2Helper::RequestAria2AddUri(_task_item_model, item);
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
std::string Aria2Helper::GetRequestCommand(std::string action, models::M3u8Task& task, models::M3u8Ts& ts)
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

std::string Aria2Helper::RequestAria2AddUri(models::M3u8Task& ti, models::M3u8Ts& ts) {
	std::string json_file = GetRequestCommand("aria2.addUri", ti, ts);
	return Aria2Helper::RequestAria2(json_file);
}
std::string Aria2Helper::RequestAria2TellStatus(models::M3u8Task& ti, models::M3u8Ts& ts)
{
	//请求状态
	std::string json_file = GetRequestCommand("aria2.tellStatus", ti, ts);
	return Aria2Helper::RequestAria2(json_file);
}
std::string Aria2Helper::RequestAria2RemoveDownloadResult(models::M3u8Task& ti, models::M3u8Ts& ts) {
	std::string json_file = GetRequestCommand("aria2.removeDownloadResult", ti, ts);
	return Aria2Helper::RequestAria2(json_file);
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
std::string Aria2Helper::RequestAria2(std::string& cmd) {
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
