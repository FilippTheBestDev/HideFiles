#include "processIoCtlCodes.h"

inline NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info) {
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
NTSTATUS ProcessIrp(IN PIRP pIrp) {
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	wchar_t* wbuf = (wchar_t*)pIrp->AssociatedIrp.SystemBuffer;
	ULONG info = 0;

	if (code == IOCTL_ADD_FILE_HIDE_RULE) {
		DbgPrint("IOCTRL_ADD_FILE_HIDE_RULE\nBuf: %ws\n", wbuf);
		size_t length;
		RtlStringCchLengthW(wbuf, 256, &length);
		_addIntoUnicodeStringsArr(wbuf, length);
	}

	else if (code == IOCTL_DEL_FILE_HIDE_RULE) {
		DbgPrint("IOCTRL_DEL_FILE_HIDE_RULE\nBuf: %ws\n", wbuf);
		size_t length;
		RtlStringCchLengthW(wbuf, 256, &length);
		_deleteFromUnicodeStringsArr(wbuf, length);
	}

	else if (code == IOCTL_CLEAR_FILE_HIDE_RULES) {
		DbgPrint("IOCTRL_CLEAR_FILE_HIDE_RULES\n");
		_clearUnicodeStringsArr();
	}

	else if (code == IOCTL_SHOW_FILE_HIDE_RULES) {
		DbgPrint("IOCTRL_SHOW_FILE_HIDE_RULES\ncbin: %d\ncbout: %d\n", cbin, cbout);

		wchar_t msg[512];
		_copyFromUnicodeStringsArrToWbuf(msg, 512);

		size_t msg_size;
		RtlStringCchLengthW(msg, 512, &msg_size);
		msg_size *= 2;

		char* dest = (char*)pIrp->AssociatedIrp.SystemBuffer;

		RtlCopyBytes(dest, msg, cbout);

		info = (ULONG)(cbout < msg_size ? cbout : msg_size) + 2;
	}

	return CompleteIrp(pIrp, STATUS_SUCCESS, info);
};