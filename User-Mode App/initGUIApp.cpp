#include "initGUIApp.h"

WNDPROC wndprocDefault = nullptr;
HWND hwndWindow = nullptr;
HWND hwndBtnClearRules = nullptr;
HWND hwndTextBoxFilename = nullptr;
HWND hwndFileOpenBtn = nullptr;
HWND hwndBtnLoadDriver = nullptr;
HWND hwndBtnUnloadDriver = nullptr;
HWND hwndTextLoadDriver = nullptr;
HWND hwndTextUnloadDriver = nullptr;
HWND hwndTextOpenFile = nullptr;
HWND hwndBtnHideEntered = nullptr;
HWND hwndListView = nullptr;
std::wstring selectedFile;
bool isDriverLoaded = false;

inline bool CALLBACK SetChildFont(HWND hwndChild, LPARAM lParam) {
    HFONT hFont = (HFONT)lParam;
    SendMessage(hwndChild, WM_SETFONT, (WPARAM)hFont, true);
    return true;
}
inline std::wstring to_wstring(const std::string& str) {
    return std::wstring(str.begin(), str.end());
};

void InitTable() {
    hwndListView = CreateWindowEx(0, WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_SHOWSELALWAYS | LVS_REPORT, 370, 10, 380, 400, hwndWindow, NULL, NULL, NULL);

    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.iSubItem = 0;
    lvColumn.pszText = (LPWSTR)L"Rules for hiding files (double-click to remove)";
    lvColumn.cx = 380;
    lvColumn.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(hwndListView, 0, &lvColumn);
};
void AppendRuleToTable(const std::wstring& rule) {
    LVITEM lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.pszText = (LPWSTR)rule.c_str();

    ListView_InsertItem(hwndListView, &lvItem);
}

void _UpdateRules(wchar_t* rules) {
    ListView_DeleteAllItems(hwndListView);

    std::wstring rules_wstr(rules);
    std::vector<std::wstring> rules_vec;
    std::wstring::size_type index, prev = 0;

    while (true) {
        index = rules_wstr.find(L"\n", prev);
        if (index == std::wstring::npos) {
            std::wstring rule = rules_wstr.substr(prev, rules_wstr.length() - prev);
            if (rule.length())
                rules_vec.push_back(rule);
            break;
        }
        std::wstring rule = rules_wstr.substr(prev, index - prev);
        rules_vec.push_back(rule);
        prev = index + 1;
    }

    printf("\n\n_UpdateRules parsed next strings:\n------------\n");
    for (auto v = rules_vec.rbegin(); v != rules_vec.rend(); v++) {
        printf("%ws\n", v->c_str());
        AppendRuleToTable(*v);
    }
    printf("------------\n\n");
}
void UpdateRules() {
    DriverShowAllRules();
}
void AddRule(wchar_t* buf) {
    DriverAddRule(std::wstring(buf));
    UpdateRules();
}
void RemoveRule(wchar_t* buf) {
    DriverDeleteRule(std::wstring(buf));
    UpdateRules();
}
void ClearRules() {
    DriverDeleteAllRules();
    UpdateRules();
}


LRESULT OnWindowClose(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (isDriverLoaded)
        if (UnloadDriver())
            isDriverLoaded = false;
    PostQuitMessage(0);
    return CallWindowProc(wndprocDefault, hwnd, message, wParam, lParam);
}
LRESULT OnButtonClick(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    wchar_t iInitialDirectory[MAX_PATH];
    SHGetSpecialFolderPath(nullptr, iInitialDirectory, CSIDL_DESKTOP, false);

    OPENFILENAME openFileName = { 0 };
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = hwnd;
    openFileName.lpstrFilter = L"All Files (*.*)\0*.*\0";
    openFileName.nFilterIndex = 1;

    std::wstring fileName(MAX_PATH, ' ');
    lstrcpyW(&fileName[0], selectedFile.c_str());
    openFileName.lpstrFile = (LPWSTR)fileName.c_str();
    openFileName.nMaxFile = MAX_PATH;
    openFileName.lpstrInitialDir = iInitialDirectory;
    openFileName.Flags = OFN_EXPLORER;

    if (GetOpenFileName(&openFileName)) {
        selectedFile = fileName;
        std::wstring fileOnlyName = GetOnlyName(fileName);
        AddRule((wchar_t*)fileOnlyName.c_str());
        SendMessage(hwndTextBoxFilename, WM_SETTEXT, 0, (LPARAM)fileOnlyName.c_str());
    }

    return 0;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CLOSE && hwnd == hwndWindow)
        return OnWindowClose(hwnd, message, wParam, lParam);

    if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED) {
        if ((HWND)lParam == hwndFileOpenBtn)
            return OnButtonClick(hwnd, message, wParam, lParam);
        if ((HWND)lParam == hwndBtnLoadDriver) {
            if (!isDriverLoaded)
                if (LoadDriver())
                    isDriverLoaded = true;
            return 0;
        }
        if ((HWND)lParam == hwndBtnUnloadDriver) {
            if (isDriverLoaded)
                if (UnloadDriver())
                    isDriverLoaded = false;
            return 0;
        }
        if ((HWND)lParam == hwndBtnHideEntered) {
            int len = GetWindowTextLengthW(hwndTextBoxFilename) + 1;
            wchar_t* text = new wchar_t[len];
            GetWindowTextW(hwndTextBoxFilename, text, len);
            *(text + len) = (wchar_t)('\0');
            AddRule(text);
            return 0;
        }
        if ((HWND)lParam == hwndBtnClearRules) {
            ClearRules();
            return 0;
        }
    }

    if (message == WM_NOTIFY) {
        if ((((LPNMHDR)lParam)->hwndFrom) == hwndListView) {
            if (((LPNMHDR)lParam)->code == NM_DBLCLK) {
                POINT cursor; // Getting the cursor position
                GetCursorPos(&cursor);
                ScreenToClient(hwndListView, &cursor);
                LVHITTESTINFO hitTestInfo;
                memset(&hitTestInfo, 0, sizeof(hitTestInfo));
                hitTestInfo.pt.x = cursor.x;
                hitTestInfo.pt.y = cursor.y;
                if (ListView_HitTest(hwndListView, &hitTestInfo) != -1) {
                    wchar_t text[255];
                    ListView_GetItemText(hwndListView, hitTestInfo.iItem, 0, text, 255);
                    RemoveRule(text);
                }
            }
        }
        return 0;
    }

    if (message == WM_PAINT) {
        PAINTSTRUCT paintStruct;
        HDC hDC;
        hDC = BeginPaint(hwndWindow, &paintStruct);
        FillRect(hDC, &paintStruct.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwndWindow, &paintStruct);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
int main() {
    hwndWindow = CreateWindowEx(0, WC_DIALOG, L"GUI Application for Driver Filter", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, nullptr, nullptr);
    wndprocDefault = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwndWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

    hwndBtnLoadDriver = CreateWindowEx(0, WC_BUTTON, L"Load Driver", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 40, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndTextLoadDriver = CreateWindowEx(0, WC_STATIC, L"Loads Driver Filter, which hides files", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 10, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndBtnUnloadDriver = CreateWindowEx(0, WC_BUTTON, L"Unload Driver", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 110, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndTextUnloadDriver = CreateWindowEx(0, WC_STATIC, L"Unloads Driver Filter, and shows all files", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 80, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndFileOpenBtn = CreateWindowEx(0, WC_BUTTON, L"Open File Browser...", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 220, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndTextOpenFile = CreateWindowEx(0, WC_STATIC, L"Select file to hide", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 190, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndTextBoxFilename = CreateWindowEx(0, WC_EDIT, L"hideme.txt", WS_BORDER | WS_CHILD | WS_VISIBLE, 10, 260, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndBtnHideEntered = CreateWindowEx(0, WC_BUTTON, L"Hide entered filename", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 295, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    hwndBtnClearRules = CreateWindowEx(0, WC_BUTTON, L"Clear all rules", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 380, 350, 30, hwndWindow, nullptr, nullptr, nullptr);
    InitTable();

    HFONT hFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
    EnumChildWindows(hwndWindow, (WNDENUMPROC)SetChildFont, (LPARAM)hFont);

    ShowWindow(hwndWindow, SW_SHOW);

    MSG message = { 0 };
    while (GetMessage(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
        UpdateWindow(hwndWindow);
    }

    return 0;
}