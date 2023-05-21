#include "loadDriver.h"

std::wstring GetOnlyName(const std::wstring& fullPath) {
	std::wstring::size_type found = fullPath.find_last_of(L"/\\");
	return fullPath.substr(found + 1, fullPath.length() - found + 2);
}
std::wstring GetOnlyPath(const std::wstring& fullPath) {
	std::wstring::size_type found = fullPath.find_last_of(L"/\\");
	return fullPath.substr(0, found);
}

bool Install(LPCWSTR driver, LPCWSTR service) {
	wchar_t szPath[MAX_PATH];

	if (!GetModuleFileName(nullptr, szPath, MAX_PATH)) {
		printf("Cannot install service (%d)\n", GetLastError());
		return false;
	}

	std::wstring fullPath(szPath);
	std::wstring driverName(driver);
	std::wstring pathToDriver = GetOnlyPath(fullPath) + L"\\" + driverName;
	printf("Path to driver: %ws\n", &(pathToDriver[0]));

	LPCWSTR driverPath = pathToDriver.c_str();

	SC_HANDLE scManager;
	SC_HANDLE srvHandle;

	scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!scManager) {
		printf("OpenSCManager failed..\n");
		return false;
	}

	//create service
	srvHandle = CreateServiceW(scManager, service, service,
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		driverPath, NULL, NULL, NULL, NULL, NULL);

	if (!srvHandle) {

		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			printf("Service already exists..\n");

			//open existing service
			srvHandle = OpenServiceW(scManager, service, SERVICE_ALL_ACCESS);
			if (!srvHandle) {
				printf("OpenService failed..\n");
				CloseServiceHandle(scManager);
				return false;
			}
		}
		else {
			CloseServiceHandle(scManager);
			printf("CreateService failed..\n");
			return false;
		}

		CloseServiceHandle(scManager);

		return FALSE;
	}

	printf("Service created!\n");

	//start service
	if (srvHandle) {
		if (StartService(srvHandle, 0, NULL) == 0) {
			if (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) {
				printf("Start Service failed..\n");
				CloseServiceHandle(srvHandle);
				CloseServiceHandle(scManager);
				return false;
			}
		}

		printf("Service started!\n");
		CloseServiceHandle(srvHandle);
	}

	CloseServiceHandle(scManager);

	return true;
}

bool Remove(LPCWSTR service) {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == INVALID_HANDLE_VALUE) {
		printf("Can't open service manager\n");
		CloseServiceHandle(hSCManager);
		return false;
	}
	SC_HANDLE hService = OpenServiceW(hSCManager, service, SERVICE_ALL_ACCESS);
	if (hService == INVALID_HANDLE_VALUE) {
		printf("Can't open driver\n");
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hService);
		return false;
	}

	SERVICE_STATUS serviceStatus;
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus)) {
		printf("Can't stop driver\n");
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hService);
		return false;
	}

	if (!DeleteService(hService)) {
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(hService);
		printf("Can't remove driver\n");
		return false;
	}

	printf("Driver was successfully stopped and removed.\n");
	CloseServiceHandle(hSCManager);
	CloseServiceHandle(hService);
	return true;
}