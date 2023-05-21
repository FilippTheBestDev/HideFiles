#ifndef PROCESS_IO_CTL_CODES_H
#define PROCESS_IO_CTL_CODES_H

#include "ntifs.h"
#include <ntstrsafe.h>
#include "hideFileAlgorithm.h"

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

inline NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info);
NTSTATUS ProcessIrp(IN PIRP pIrp);

#endif