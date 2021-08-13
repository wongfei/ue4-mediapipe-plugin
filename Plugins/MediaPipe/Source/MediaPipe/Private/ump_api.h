#pragma once

#include <cstdint>

//
// Log
//

enum class EUmpVerbosity
{
	Error = 0,
	Warning,
	Info,
	Debug
};

class IUmpLog
{
protected:
	virtual ~IUmpLog() {}

public:
	virtual void Println(EUmpVerbosity verbosity, const char* msg) const {}
};

//
// Object
//

class IUmpObject
{
protected:
	virtual ~IUmpObject() {}

public:
	struct Dtor { void operator()(IUmpObject* obj) { obj->Release(); } };
	virtual void Release() = 0;
	virtual void AddRef() = 0;
};

//
// Context
//

class IUmpContext : public IUmpObject
{
public:
	virtual void SetLog(IUmpLog* log) = 0;
	virtual void SetResourceDir(const char* resource_dir) = 0;
	virtual class IUmpPipeline* CreatePipeline() = 0;
};

extern "C"
{
	IUmpContext* UmpCreateContext();
	typedef IUmpContext* UmpCreateContext_Proto();
}

//
// Pipeline
//

class IUmpPipeline : public IUmpObject
{
public:
	virtual void SetGraphConfiguration(const char* filename) = 0;
	virtual void SetCaptureFromFile(const char* filename) = 0;
	virtual void SetCaptureParams(int cam_id, int cam_api, int cam_resx, int cam_resy, int cam_fps) = 0;
	virtual void SetOverlay(bool overlay) = 0;
	virtual class IUmpObserver* CreateObserver(const char* stream_name) = 0;
	virtual bool Start() = 0;
	virtual void Stop() = 0;

	// debug
	virtual void LogProfilerStats() = 0;
	virtual uint64_t GetLastFrameId() = 0;
	virtual double GetLastFrameTimestamp() = 0;
};

//
// Observer
//

class IUmpPacketCallback
{
public:
	virtual void OnUmpPresence(class IUmpObserver* observer, bool present) = 0;
	virtual void OnUmpPacket(class IUmpObserver* observer) = 0;
};

class IUmpObserver : public IUmpObject
{
public:
	virtual void SetPacketCallback(IUmpPacketCallback* callback) = 0;
	virtual size_t GetMessageType() = 0;
	virtual const void* const GetData() = 0;
};
