#ifndef CONTROL_DRIVER_H
#define CONTROL_DRIVER_H

#include <iostream>
#include <windows.h>
#include <winioctl.h>
#include "winsvc.h"
#include <exception>
#include <tchar.h>
#include <string>

#include "loadDriver.h"
#include "initGUIApp.h"

// додати файл до списку приховуваних, в *inBuf буде iм'я файлу
#define IOCTL_ADD_FILE_HIDE_RULE CTL_CODE(\
    0x8888, 0x880, METHOD_BUFFERED, FILE_ANY_ACCESS)
// видалити файл зi списку приховуваних, в *inBuf буде iм'я файлу
#define IOCTL_DEL_FILE_HIDE_RULE CTL_CODE(\
    0x8888, 0x881, METHOD_BUFFERED, FILE_ANY_ACCESS)
// видалити УСI файли зi списку приховуваних
#define IOCTL_CLEAR_FILE_HIDE_RULES CTL_CODE(\
    0x8888, 0x882, METHOD_BUFFERED, FILE_ANY_ACCESS)
// показати файли зi списку приховуваних
#define IOCTL_SHOW_FILE_HIDE_RULES CTL_CODE(\
    0x8888, 0x883, METHOD_BUFFERED, FILE_ANY_ACCESS)

void SendIoCtlCode(DWORD ioCtlCode, PWCHAR pInBuf, DWORD inBufSize);

void DriverAddRule(const std::wstring& hideRuleStr);
void DriverDeleteRule(const std::wstring& hideRuleStr);
void DriverDeleteAllRules();
void DriverShowAllRules();

bool LoadDriver();
bool UnloadDriver();

#endif