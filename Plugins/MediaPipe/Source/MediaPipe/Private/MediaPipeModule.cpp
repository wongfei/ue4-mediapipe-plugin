#include "MediaPipeModule.h"
#include "MediaPipeShared.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogMediaPipe);

IMPLEMENT_MODULE(FMediaPipeModule, MediaPipe)

class UmpLog : public IUmpLog
{
public:
	void Println(EUmpVerbosity verbosity, const char* msg) const override
	{
		#define LOG_IMPL(v, msg) PLUGIN_LOG(v, TEXT("%S"), msg)
		switch (verbosity)
		{
			case EUmpVerbosity::Error: LOG_IMPL(Error, msg); break;
			case EUmpVerbosity::Warning: LOG_IMPL(Warning, msg); break;
			case EUmpVerbosity::Info: LOG_IMPL(Display, msg); break;
			default: LOG_IMPL(Verbose, msg); break;
		}
	}
};

static UmpLog UmpLogger;

void FMediaPipeModule::StartupModule()
{
	PLUGIN_TRACE;

	#define UMP_BIN_ARCH UMP_STRINGIZE(UBT_COMPILED_PLATFORM)

	#define UMP_LIB_NAME "ump_shared"

	#if PLATFORM_WINDOWS
		#define UMP_LIB_EXT ".dll"
	#else
		#define UMP_LIB_EXT ".so"
	#endif

	auto Plugin = IPluginManager::Get().FindPlugin(TEXT(PLUGIN_NAME));
	auto PluginBaseDir = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	PLUGIN_LOG_INFO(TEXT("PluginBaseDir: %s"), *PluginBaseDir);

	FString BinariesDir, DataDir;
	#if WITH_EDITOR
		BinariesDir = FPaths::Combine(*PluginBaseDir, TEXT("Binaries"), TEXT(UMP_BIN_ARCH));
		DataDir = FPaths::Combine(*PluginBaseDir, TEXT("ThirdParty/mediapipe/Data"));
	#else
		BinariesDir = FPlatformProcess::BaseDir();
		DataDir = BinariesDir;
	#endif
	PLUGIN_LOG_INFO(TEXT("BinariesDir: %s"), *BinariesDir);
	PLUGIN_LOG_INFO(TEXT("DataDir: %s"), *DataDir);

	FString LibPath = FPaths::Combine(*BinariesDir, TEXT(UMP_LIB_NAME) TEXT(UMP_LIB_EXT));
	LibUmp = FPlatformProcess::GetDllHandle(*LibPath);
	if (!LibUmp)
	{
		PLUGIN_LOG(Error, TEXT("Unable to load: %s"), *LibPath);
	}
	else
	{
		CreateContextPtr = FPlatformProcess::GetDllExport(LibUmp, TEXT("DYN_UmpCreateContext"));
		if (!CreateContextPtr)
		{
			PLUGIN_LOG(Error, TEXT("Export not found: DYN_UmpCreateContext"));
		}
		else
		{
			Context = ((UmpCreateContext_Proto*)CreateContextPtr)();
			if (!Context)
			{
				PLUGIN_LOG(Error, TEXT("Unable to create IUmpContext"));
			}
			else
			{
				Context->SetLog(&UmpLogger);
				Context->SetResourceDir(TCHAR_TO_ANSI(*DataDir));
			}
		}
	}
}

void FMediaPipeModule::ShutdownModule()
{
	PLUGIN_TRACE;

	if (Context)
	{
		Context->SetLog(nullptr);
		Context->Release();
		Context = nullptr;
	}

	if (LibUmp)
	{
		FPlatformProcess::FreeDllHandle(LibUmp);
		LibUmp = nullptr;
	}
}

IUmpPipeline* FMediaPipeModule::CreatePipeline()
{
	PLUGIN_TRACE;

	if (!Context)
	{
		PLUGIN_LOG(Error, TEXT("Invalid state: IUmpContext"));
		return nullptr;
	}

	auto Pipeline = Context->CreatePipeline();
	if (!Pipeline)
	{
		PLUGIN_LOG(Error, TEXT("Unable to create IUmpPipeline"));
	}

	return Pipeline;
}
