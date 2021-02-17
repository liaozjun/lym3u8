#include "pch.h"
#include "m3u8_task.h"
namespace models {
	

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
	
 

}