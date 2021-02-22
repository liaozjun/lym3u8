#pragma once
class HttpServerRunner : public nbase::SupportWeakCallback
{
public:
	void RunMongooseLoop(std::string url);
	void StopMongooseLoop();
private:
	HINSTANCE _hInst;
	std::string _url;
};

