#include "base.h"

char* logfilename;
void openlog(char* filename, int level, int unknown) {
  logfilename = filename;
}

int WriteLog(char* str) {
  HANDLE logFile = CreateFileA(logfilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (logFile != INVALID_HANDLE_VALUE) {
    CTime ct = CTime::GetCurrentTime();
    char log[512];
    DWORD len = sprintf_s(log, "%d-%02d-%02d %02d:%02d:%02d %s\n", ct.GetYear(), ct.GetMonth(), ct.GetDay(), ct.GetHour(), ct.GetMinute(), ct.GetSecond(), str);
    SetFilePointer(logFile, 0, NULL, FILE_END);
    WriteFile(logFile, log, len, &len, NULL);
  }

  CloseHandle(logFile);
  return 0;
}