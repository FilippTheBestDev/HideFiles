#ifndef LOAD_DRIVER_H
#define LOAD_DRIVER_H

#include <stdio.h>
#include <Windows.h>
#include <string>

std::wstring GetOnlyName(const std::wstring& fullPath);
std::wstring GetOnlyPath(const std::wstring& fullPath);
bool Install(LPCWSTR driver, LPCWSTR service);
bool Remove(LPCWSTR service);

#endif