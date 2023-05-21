#include "controlDriver.h"

void SendIoCtlCode(DWORD ioCtlCode, PWCHAR pInBuf, DWORD inBufSize) {
    WCHAR outBuf[512];
    DWORD bytesReturned;

    HANDLE hHandle = CreateFile(
        _T("\\\\.\\HideFiles"),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hHandle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Can't get handle to driver\n");

    if (!DeviceIoControl(hHandle, ioCtlCode, pInBuf, inBufSize, (PWCHAR)outBuf, sizeof(outBuf), &bytesReturned, NULL))
        throw std::runtime_error("Driver communication error\n");

    if (bytesReturned != 0) {
        printf("Current rules:\n---------\n%ws\n---------\n", (PWCHAR)outBuf);
        _UpdateRules((PWCHAR)outBuf);
    }
}

void DriverAddRule(const std::wstring& hideRuleStr) {
    DWORD size = (hideRuleStr.length() + 3) * 2; // Adding size of '\0' symbol & take into consideration UNICODE size of character
    SendIoCtlCode(IOCTL_ADD_FILE_HIDE_RULE, (PWCHAR)hideRuleStr.c_str(), size);
}
void DriverDeleteRule(const std::wstring& hideRuleStr) {
    DWORD size = (hideRuleStr.length() + 3) * 2; // Adding size of '\0' symbol & take into consideration UNICODE size of character
    SendIoCtlCode(IOCTL_DEL_FILE_HIDE_RULE, (PWCHAR)hideRuleStr.c_str(), size);
}
void DriverDeleteAllRules() {
    WCHAR inBuf;
    SendIoCtlCode(IOCTL_CLEAR_FILE_HIDE_RULES, &inBuf, sizeof(inBuf) /* 2B */);
}
void DriverShowAllRules() {
    WCHAR inBuf;
    SendIoCtlCode(IOCTL_SHOW_FILE_HIDE_RULES, &inBuf, sizeof(inBuf) /* 2B */);
}

bool LoadDriver() {
    const wchar_t driverName[] = L"HideFiles.sys";
    const wchar_t serviceName[] = L"HideFilesSrv";

    return Install(driverName, serviceName);
}

bool UnloadDriver() {
    const wchar_t serviceName[] = L"HideFilesSrv";

    return Remove(serviceName);
}