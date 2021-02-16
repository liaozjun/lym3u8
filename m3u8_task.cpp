#include "pch.h"
#include "m3u8_task.h"

#include "third_party/curl/include/curl/curl.h"
#include "third_party/jsoncpp/include/json/json.h"

namespace models {
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

	M3u8Task::M3u8Task(std::string title, std::string url, std::string content) {
		_id = 0;
		this->_title = title;
		this->_url = url;
		this->_content = content;
		create_time = 0;
		end_time = 0;
	}
	M3u8Task::~M3u8Task() {
		this->_title = "";
		this->_id = 0;
		this->_status = models::M3u8Task::Status::Saved;
		this->_content = "";
		create_time = 0;
		end_time = 0;
	}
	void M3u8Task::LoadDbInit(int64 id, std::string title, std::string url, models::M3u8Task::Status status, std::string folder_name, std::string content,
		int64_t ct,int64_t et) {
		this->_id = id;
		this->_title = title;
		this->_url = url;
		this->_status = status;
		this->_folder_name = folder_name;
		this->_content = content;
		create_time = ct;
		end_time = et;
	}

	bool M3u8Task::ProcessContext(std::list<std::string>& urls, std::list<std::string>& allm3u8) {
		std::string::size_type first_extinf = this->_content.find("#EXTINF");
		std::string::size_type last_extinf = this->_content.rfind("#EXT-X-ENDLIST");
		std::string m3u8substr = this->_content.substr(first_extinf, last_extinf - first_extinf);
		std::string beginstr = this->_content.substr(0, first_extinf);
		std::string endstr = this->_content.substr(last_extinf, this->_content.size());

		std::list<std::string> splits;
		nbase::StringTokenize(m3u8substr, "\n", splits);

		std::string::size_type uen = this->_url.rfind("/");
		std::string urltmp = this->_url.substr(0, uen + 1);

		//std::list<std::string> urlss;
		//std::list<std::string> urllocalhost;
		allm3u8.push_back(beginstr);

		auto that = this;
		auto fe = [that, &urls, &urltmp, &allm3u8](std::string str) {
			std::string::size_type ex = str.find("#EXTINF");
			if (ex == str.npos) {
				if (str.find("http") == str.npos) {
					urls.push_back(urltmp + str);
				}
				else {
					urls.push_back(str);
				}
				std::string::size_type lastof = str.rfind("/");
				std::string::size_type lastofwhy = str.rfind("?");
				int pos = 0;
				if (lastofwhy == str.npos) {
					pos = str.size();
				}
				else {
					pos = lastofwhy;
				}
				std::string lf = str.substr(lastof + 1, pos - lastof - 1);
				allm3u8.push_back(lf);
			}
			else {
				allm3u8.push_back(str);
			}
		};
		std::for_each(splits.begin(), splits.end(), fe);
		allm3u8.push_back(endstr);
		return true;
	}
	
	void M3u8Task::SendAria2() 
	{
		size_t len = _details_ts.size();
		
		for (auto it = _details_ts.begin(); it != _details_ts.end(); it++) 
		{
			Json::FastWriter writer;
			Json::Value value;
			Json::Value array;

			//url
			Json::Value array1;
			array1.append(it->_url);
			array.append(array1);
			//dir
			std::string theme_dir = nbase::UTF16ToUTF8(nbase::win32::GetCurrentModuleDirectory());
			Json::Value parms;
			parms["dir"] = theme_dir+"m3u8\\" + this->_folder_name;
			array.append(parms);

			value["jsonrpc"] = "2.0";
			value["id"] = "qwer";
			value["method"] = "aria2.addUri";
			value["params"] = array;

			std::string json_file = writer.write(value);

			CURLcode res;
			CURL* curl = curl_easy_init();
			if (NULL == curl)
			{
				return;// CURLE_FAILED_INIT;
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
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_file.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_file.size());//设置上传json串长度,这个设置可以忽略
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
			res = curl_easy_perform(curl);
			curl_slist_free_all(headers); /* free the list again */
			curl_easy_cleanup(curl);
			
			//json parse strResponse result 
			Json::Value root;
			Json::Reader jr;
			jr.parse(strResponse, root);
			it->_status = models::M3u8Ts::Status::Downloading;
			//update ts

			break;
		}
	}

}