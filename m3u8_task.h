#pragma once

namespace models {
	class M3u8Ts;
	class M3u8Task
	{
	public:
		enum Status
		{
			//未下载
			Saved = 0,
			//等待下载
			WaitingForDownload = 1,
			//下载中
			Downloading = 2,
			//完成下载
			DownloadComplete = 3,
			//暂停
			Pause = 4,
			//重新下载错误的TS
			ReDownloadErrorTs = 5
		};
		M3u8Task() {
			_id = 0;
			create_time = 0;
			end_time = 0; 
		};
		M3u8Task(const models::M3u8Task& t) {
			this->_id = t._id;
			this->_title = t._title;
			this->_url = t._url;
			this->_status = t._status;
			this->_folder_name = t._folder_name;
			this->_content = t._content;
			this->create_time = t.create_time;
			this->end_time = t.end_time;
			this->count = t.count;
			this->count_downloading = t.count_downloading;
			this->count_complete = t.count_complete;
			this->count_error = t.count_error;
		}
		M3u8Task(std::string title, std::string url, std::string content);
		~M3u8Task();
		void LoadDbInit(int64 id, std::string title, std::string url, models::M3u8Task::Status status, std::string folder_name
			, std::string content, int64_t ct, int64_t et, int count, int count_downloading, int count_complete, int count_error);
		bool ProcessContext(std::list<std::string>& urls, std::list<std::string>& allm3u8);
		 
	public:
		int64 _id;//	INTEGER,
		std::string _title;// "	TEXT,
		std::string _url;
		Status _status;// INTEGER DEFAULT 0,
		std::string _folder_name;
		std::string 	_content;
		int64_t create_time;
		int64_t end_time;
		
		int count;
		int count_downloading;
		int count_complete;
		int count_error;

		std::list<M3u8Ts> _details_ts;
	};

	class M3u8Ts {
	public:
		enum Status
		{
			//未下载
			Saved = 0,
			//下载中
			Downloading = 1,
			//完成下载
			DownloadComplete = 2
		};
		M3u8Ts() {};
		~M3u8Ts() {};
		int64 _id;
		int64 _m3u8_task_id;
		std::string _aria2_result;
		Status _status;
		std::string _url;
		int64_t _create_time;
		std::string	errorCode;
		std::string errorMessage;
	};
}