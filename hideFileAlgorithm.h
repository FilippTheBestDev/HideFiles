#ifndef HIDE_FILE_ALGORITHM_H
#define HIDE_FILE_ALGORITHM_H

#include "ntifs.h"
#include <ntstrsafe.h>

#define NtQuery ZwQueryDirectoryFile
#define LastEntryOffset 0

extern unsigned int unicodeStringsArr_curIndex;
extern const unsigned int unicodeStringsArr_length;
extern wchar_t unicodeStringsArr[][256];

void _setAtStringsArr(wchar_t* wbuf, size_t wbuf_size, size_t pos);
void _clearUnicodeStringsArr();
void _addIntoUnicodeStringsArr(wchar_t* wbuf, size_t wbuf_size);
void _deleteFromUnicodeStringsArr(wchar_t* wbuf, size_t wbuf_size);
void _copyFromUnicodeStringsArrToWbuf(wchar_t* wbuf, size_t max_size);

struct NtList {
	ULONG NextEntryOffset;
};

struct NtQueryDirParams {
	HANDLE FileHandle;
	HANDLE Event;
	PIO_APC_ROUTINE ApcRoutine;
	PVOID ApcContext;
	PIO_STATUS_BLOCK IoStatusBlock;
	PVOID FileInformation;
	ULONG FileInformationLength;
	FILE_INFORMATION_CLASS FileInformationClass;
	BOOLEAN ReturnSingleEntry;
	PUNICODE_STRING FileName;
	BOOLEAN RestartScan;
};

typedef NTSTATUS(*OriginalHandlerWrapperPtr)(const NtQueryDirParams& params);

struct HideParams {
	const NtQueryDirParams& callParams;
	OriginalHandlerWrapperPtr wrapperPtr;
};

inline bool IsEntryLast(NtList* pList);
inline NtList* GetNextEntryPointer(NtList* list);
inline size_t GetListSize(NtList* list);
inline void CutNextEntryByFakeOffset(NtList* list);
template<class InfoType> inline void CutFromListByFakeOffset_If(NtList* list, const HideParams& params);
bool IsFileNameInRules(const wchar_t* s, size_t size, const NtQueryDirParams& params);
template<class InfoType> inline bool CheckFileEntry(NtList* list, const HideParams& params);
inline void ShiftBuffer(NtList* pList, NtList* newBegin);
template<class InfoType> inline NTSTATUS FirstEntryProcessor(const HideParams& params);
template<class InfoType> inline NTSTATUS NextEntryProcessor(const HideParams& params);
template<class InfoType> inline NTSTATUS HideFile(const HideParams& params);

ULONG ClearWPBit();
void SetWPBit(ULONG CR0Reg);

typedef struct SystemServiceTable {
	UINT32* ServiceTable;
	UINT32* CounterTable;
	UINT32 ServiceLimit;
	UINT32* ArgumentTable;
} SST;

typedef NTSTATUS(*NtQueryPrototype) (
	IN HANDLE FileHandle,
	IN HANDLE Event OPTIONAL,
	IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
	IN PVOID ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformation,
	IN ULONG FileInformationLength,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN BOOLEAN ReturnSingleEntry,
	IN PUNICODE_STRING FileName OPTIONAL,
	IN BOOLEAN RestartScan);

extern "C" __declspec(dllimport) SST KeServiceDescriptorTable;
extern ULONG oldWP;
extern NtQueryPrototype oldNtQuery;

NTSYSAPI NTSTATUS NTAPI NtQuery(
	IN HANDLE FileHandle,
	IN HANDLE Event OPTIONAL,
	IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
	IN PVOID ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformation,
	IN ULONG FileInformationLength,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN BOOLEAN ReturnSingleEntry,
	IN PUNICODE_STRING FileName OPTIONAL,
	IN BOOLEAN RestartScan);
NTSTATUS OriginalHandlerWrapper(const NtQueryDirParams& params);
NTSTATUS Hook_NtQuery(
	IN HANDLE FileHandle,
	IN HANDLE Event OPTIONAL,
	IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
	IN PVOID ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformation,
	IN ULONG FileInformationLength,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN BOOLEAN ReturnSingleEntry,
	IN PUNICODE_STRING FileName OPTIONAL,
	IN BOOLEAN RestartScan);
PULONG HookSSDT(PUCHAR syscall, PUCHAR hookaddr);
void RestoreNtQuery();
void ModifyNtQuery();

#endif