#pragma once

#include <algorithm>
#include <vector>
#include <memory>

inline uint8_t CkParseHex(uint8_t Val)
{
	if (Val >= '0' && Val <= '9') return (Val - '0');
	if (Val >= 'a' && Val <= 'f') return (Val - 'a') + 10;
	if (Val >= 'A' && Val <= 'F') return (Val - 'A') + 10;
	return 0;
}

inline uint8_t CkParseByte(const char* Str)
{
	uint8_t hi = CkParseHex((uint8_t)Str[0]);
	uint8_t lo = CkParseHex((uint8_t)Str[1]);
	return ((hi << 4) | lo);
}

inline std::vector<uint8_t> CkParseByteArray(const char* Str)
{
	std::vector<uint8_t> Result;
	if (Str)
	{
		const size_t Len = strlen(Str);
		if (Len)
		{
			std::vector<uint8_t> Tmp;
			Tmp.reserve(Len);

			for (size_t i = 0; i < Len; i++)
			{
				if (isalnum(Str[i]))
					Tmp.push_back(Str[i]);
			}

			const size_t NumBytes = Tmp.size() / 2;
			if (NumBytes)
			{
				Result.resize(NumBytes);
				for (size_t i = 0; i < NumBytes; i++)
				{
					uint8_t hi = CkParseHex(Tmp[i*2]);
					uint8_t lo = CkParseHex(Tmp[(i*2)+1]);
					Result[i] = ((hi << 4) | lo);
				}
			}
		}
	}
	return std::move(Result);
}

template<const uint8_t Wildcard>
struct CkWildcard
{
	inline bool operator()(const uint8_t &a, const uint8_t &b) const
	{
		return (a == b || b == Wildcard);
	}
};

typedef CkWildcard<0xCC> CkWildcardCC; // Interrupt Type 3
typedef CkWildcard<0xCE> CkWildcardCE; // Interrupt if Overflow

#define CK_PAGE_EXECUTE_RWC (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

template<typename TCompare>
inline DWORD CkFindPatternIntern(HANDLE Process, const std::vector<uint8_t>& Pattern, size_t Limit, std::vector<uint8_t*>& Result)
{
	if (!Process)
		Process = GetCurrentProcess();

	SYSTEM_INFO SysInfo;
	ZeroMemory(&SysInfo, sizeof(SysInfo));
	GetSystemInfo(&SysInfo);

	uint8_t* MaxAddress = (uint8_t*)SysInfo.lpMaximumApplicationAddress;
	uint8_t* Ptr = (uint8_t*)SysInfo.lpMinimumApplicationAddress;

	TCompare Compare;

	while (Ptr < MaxAddress)
	{
		MEMORY_BASIC_INFORMATION MemInfo;
		if (VirtualQueryEx(Process, Ptr, &MemInfo, sizeof(MemInfo)) != sizeof(MemInfo))
		{
			return GetLastError();
		}

		if ((MemInfo.Protect & CK_PAGE_EXECUTE_RWC) && ((MemInfo.Protect & PAGE_GUARD) == 0) && ((MemInfo.Protect & PAGE_NOACCESS) == 0))
		{
			uint8_t* RegionPos = (uint8_t*)MemInfo.BaseAddress;
			uint8_t* RegionEnd = RegionPos + MemInfo.RegionSize;

			while ((RegionPos = std::search(RegionPos, RegionEnd, Pattern.begin(), Pattern.end(), Compare)) != RegionEnd)
			{
				Result.push_back(RegionPos);

				if (Limit && Result.size() >= Limit)
					return 0;

				RegionPos++;
			}
		}

		Ptr += MemInfo.RegionSize;
	}

	return 0;
}

inline DWORD CkProtectWriteMemory(HANDLE Process, const std::vector<uint8_t>& Data, PVOID Addr, SIZE_T Offset)
{
	if (!Process)
		Process = GetCurrentProcess();

	DWORD Prot = 0;
	if (!VirtualProtectEx(Process, Addr, Data.size(), PAGE_EXECUTE_READWRITE, &Prot))
		return GetLastError();

	memcpy((PVOID)((UINT64)Addr + (UINT64)Offset), &Data[0], Data.size());

	DWORD Prot2 = 0;
	VirtualProtectEx(Process, Addr, Data.size(), Prot, &Prot2);

	return 0;
} 
