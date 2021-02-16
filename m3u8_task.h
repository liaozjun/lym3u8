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
			DownloadComplete = 3
		};
		M3u8Task() {
			create_time = 0;
			end_time = 0; 
		};
		M3u8Task(std::string title, std::string url, std::string content);
		~M3u8Task();
		void LoadDbInit(int64 id, std::string title, std::string url, models::M3u8Task::Status status, std::string folder_name, std::string content, int64_t ct, int64_t et);
		bool ProcessContext(std::list<std::string>& urls, std::list<std::string>& allm3u8);
		void SendAria2();
	public:
		int64 _id;//	INTEGER,
		std::string _title;// "	TEXT,
		std::string _url;
		Status _status;// INTEGER DEFAULT 0,
		std::string _folder_name;
		std::string 	_content;
		int64_t create_time;
		int64_t end_time;
		 

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
	};
}