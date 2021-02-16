#pragma once
class HttpServerRunner : public nbase::SupportWeakCallback
{
public:
	void RunMongooseLoop();
	void StopMongooseLoop();
private:
	HINSTANCE _hInst;
};

