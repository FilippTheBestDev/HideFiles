#ifndef INIT_GUI_H
#define INIT_GUI_H

#ifndef UNICODE
#define UNICODE
#endif

#include <map>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <ShlObj.h>

#include "controlDriver.h"
#include "loadDriver.h"

extern WNDPROC wndprocDefault;
extern HWND hwndWindow;
extern HWND hwndFileOpenBtn;
extern HWND hwndBtnLoadDriver;
extern HWND hwndBtnUnloadDriver;
extern HWND hwndTextLoadDriver;
extern HWND hwndTextUnloadDriver;
extern HWND hwndText_1;
extern HWND hwndText_2;
extern HWND hwndListView;
extern std::wstring selectedFile;

inline bool CALLBACK SetChildFont(HWND hwndChild, LPARAM lParam);
inline std::wstring to_wstring(const std::string& str);
void InitTable();
void AppendRuleToTable(const std::wstring& rule);

void _UpdateRules(wchar_t* rules);
void UpdateRules();

void AddRule(wchar_t* buf);
void RemoveRule(wchar_t* buf);

LRESULT OnWindowClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT OnButtonClick(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int main();

#endif