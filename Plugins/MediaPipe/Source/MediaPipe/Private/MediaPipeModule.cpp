#include "MediaPipeModule.h"
#include "MediaPipeShared.h"
#include "Misc/Paths.h"
#include "Runtime/Launch/Resources/Version.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include "Hakz.h"
#endif

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
			//default: LOG_IMPL(Verbose, msg); break;
			default: LOG_IMPL(Display, msg); break;
		}
	}
};

static UmpLog UmpLogger;

// SUPER LAME FIX for deadlock in FModuleTrace::OnDllLoaded (hello EPIC? plz don't use LdrRegisterDllNotification)
void FixDeadlock()
{
	#if (ENGINE_MAJOR_VERSION == 5)

	const char* Variants[] = {
		// 5.0.0
		"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B 05 B5 C6 B5 00", // dev editor (unrealeditor-core.dll)
		"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B 05 CC CC CC CC 48 33 C4 48 89 45 17 4D 63 68 3C 33 C0", // dev game
	};
	const int NumVariants = sizeof(Variants) / sizeof(Variants[0]);

	auto Process = GetCurrentProcess();

	for (int i = 0; i < NumVariants; ++i)
	{
		auto Pattern = CkParseByteArray(Variants[i]);
		std::vector<uint8_t*> Loc;
		auto Status = CkFindPatternIntern<CkWildcardCC>(Process, Pattern, 2, Loc);
		PLUGIN_LOG_INFO(TEXT("FindPattern Id=%d Status=%d Count=%d [%s]"), i, (int)Status, (int)Loc.size());

		if (Status == 0 && Loc.size() == 1)
		{
			PLUGIN_LOG_INFO(TEXT("FModuleTrace::OnDllLoaded -> %p"), Loc[0]);
			auto Fix = CkParseByteArray("C3");
			Status = CkProtectWriteMemory(Process, Fix, Loc[0], 0);
			PLUGIN_LOG_INFO(TEXT("WriteMemory -> %u"), (unsigned int)Status);
			return;
		}
	}

	PLUGIN_LOG(Warning, TEXT("FModuleTrace::OnDllLoaded NOT FOUND"));

	#endif
}

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

	FixDeadlock();

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

	PLUGIN_LOG_INFO(TEXT("GetDllHandle: %s"), *LibPath);
	LibUmp = FPlatformProcess::GetDllHandle(*LibPath);
	PLUGIN_LOG_INFO(TEXT("-> %p"), LibUmp);

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
