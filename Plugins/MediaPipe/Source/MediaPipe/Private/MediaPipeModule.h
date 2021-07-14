#pragma once

#include "IMediaPipeModule.h"
#include "Interfaces/IPluginManager.h"

class FMediaPipeModule : public IMediaPipeModule
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
	class IUmpPipeline* CreatePipeline() override;

private:
	void* LibUmp = nullptr;
	void* CreateContextPtr = nullptr;
	class IUmpContext* Context = nullptr;
};
