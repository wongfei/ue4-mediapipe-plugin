#pragma once

#include "CoreMinimal.h"
#include "ump_api.h"

#define PLUGIN_NAME "MediaPipe"

DECLARE_LOG_CATEGORY_EXTERN(LogMediaPipe, Log, All);

#define PLUGIN_LOG(Verbosity, Format, ...) UE_LOG(LogMediaPipe, Verbosity, Format, __VA_ARGS__)
#define PLUGIN_LOG_INFO(Format, ...) UE_LOG(LogMediaPipe, Display, Format, __VA_ARGS__)
#define PLUGIN_TRACE PLUGIN_LOG_INFO(TEXT("%S"), __FUNCTION__)

#define UMP_STRINGIZE_INNER(x) #x
#define UMP_STRINGIZE(x) UMP_STRINGIZE_INNER(x)

struct UmpMat4
{
	union
	{
		float arr[16];
		float m[4][4];
	};
};

class TypeInfo {
public:
	template <typename T>
	static const TypeInfo& Get() {
		//static TypeInfo* static_type_info = new TypeInfo(typeid(T));
		//return *static_type_info;
		static TypeInfo static_type_info(typeid(T));
		return static_type_info;
	}
	size_t hash_code() const { return info_.hash_code(); }

private:
	TypeInfo(const std::type_info& info) : info_(info) {}
	TypeInfo(const TypeInfo&) = delete;

private:
	const std::type_info& info_;
};

template<typename T>
inline bool UmpCompareType(size_t type)
{
	return (type == TypeInfo::Get<T>().hash_code());
}

template<typename T>
inline bool UmpCompareType(class IUmpObserver* observer)
{
	return (observer->GetMessageType() == TypeInfo::Get<T>().hash_code());
}

template<typename T>
inline const T& UmpCastPacket(const void * const pk_data)
{
	return *((const T * const)pk_data);
}

template<typename T>
inline void UmpSafeRelease(T*& obj) { if (obj) { obj->Release(); obj = nullptr; } }

template<typename T>
inline void Grow(TArray<T>& Storage, int Cap)
{
	if (Storage.Num() < Cap)
		Storage.SetNum(Cap);
}

inline FVector Scale3D(const FVector& V, const FVector& S)
{
	return FVector(V.X * S.X, V.Y * S.Y, V.Z * S.Z);
}

inline FVector ShuffleAxes(const FVector& V, int X, int Y, int Z)
{
	constexpr int Id[] = {0, 1, 2, 0, 1, 2};
	constexpr float Sign[] = {1, 1, 1, -1, -1, -1};

	const float* Vptr = &V.X;
	FVector Out;
	Out.X = Vptr[Id[X]] * Sign[X];
	Out.Y = Vptr[Id[Y]] * Sign[Y];
	Out.Z = Vptr[Id[Z]] * Sign[Z];
	return Out;
}
