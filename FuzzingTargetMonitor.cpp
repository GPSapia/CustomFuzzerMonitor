#include <Windows.h>
#include <iostream>
#include <strsafe.h>

#define _CRT_NON_CONFORMING_WCSTOK 

STARTUPINFO si;
PROCESS_INFORMATION pi;
DEBUG_EVENT event;
LPWSTR* cmdLineArgs;

bool debuggerAttached = false;                              //this will be useful if I want to use this also to collect coverage through breakpoints

//========= Command Line Arguments ==============

LPWSTR targetProgram = NULL;
LPWSTR corpusDirectory = NULL;                                 //if we want to take input from files
LPWSTR inputStringsFile = NULL;                                     //if the input is to be passed directly from command line

//========= Command Line Arguments ==============

//Start a process attaching to it
int startAndDebugProcess(wchar_t* processCmdLine)
{
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	return CreateProcess(NULL, processCmdLine, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi);
}

bool handleException(EXCEPTION_RECORD record, DWORD* dwContinueStatus)
{
    DWORD exceptionCode = record.ExceptionCode;

    switch (exceptionCode)
    {
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            printf("EXCEPTION_ARRAY_BOUNDS_EXCEEDED at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
        case EXCEPTION_ACCESS_VIOLATION:
            printf("EXCEPTION_ACCESS_VIOLATION at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            std::cout << "EXCEPTION_DATATYPE_MISALIGNMENT\n";
            return false;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            std::cout << "EXCEPTION_FLT_DENORMAL_OPERAND\n";
            return false;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            std::cout << "EXCEPTION_FLT_DIVIDE_BY_ZERO\n";
            return false;
        case EXCEPTION_FLT_INEXACT_RESULT:
            std::cout << "EXCEPTION_FLT_INEXACT_RESULT\n";
            return false;
        case EXCEPTION_FLT_INVALID_OPERATION:
            std::cout << "EXCEPTION_FLT_INVALID_OPERATION\n";
            return false;
        case EXCEPTION_FLT_OVERFLOW:
            std::cout << "EXCEPTION_FLT_OVERFLOW\n";
            return false;
        case EXCEPTION_FLT_STACK_CHECK:
            printf("EXCEPTION_FLT_STACK_CHECK at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
        case EXCEPTION_FLT_UNDERFLOW:
            std::cout << "EXCEPTION_FLT_UNDERFLOW\n";
            return false;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            printf("EXCEPTION_ILLEGAL_INSTRUCTION at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
        case EXCEPTION_IN_PAGE_ERROR:
            printf("EXCEPTION_IN_PAGE_ERROR at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            std::cout << "EXCEPTION_INT_DIVIDE_BY_ZERO\n";
            return false;
        case EXCEPTION_INT_OVERFLOW:
            std::cout << "EXCEPTION_INT_OVERFLOW: INVESTIGATE!\n";
            return false;
        case EXCEPTION_INVALID_DISPOSITION:
            std::cout << "EXCEPTION_INVALID_DISPOSITION\n";
            return false;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            printf("EXCEPTION_NONCONTINUABLE_EXCEPTION at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
        case EXCEPTION_PRIV_INSTRUCTION:
            std::cout << "EXCEPTION_PRIV_INSTRUCTION\n";
            return false;
        case EXCEPTION_SINGLE_STEP: 
            return false;
        case EXCEPTION_STACK_OVERFLOW:
            printf("EXCEPTION_STACK_OVERFLOW at address: %x\n", record.ExceptionAddress);
            *dwContinueStatus = DBG_CONTINUE;
            return true;
    }
}


//If an interesting exception occurred, stop the execution of the program
bool dispatchDebugEvent(DWORD* dwContinueStatus)
{
    *dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    bool stopExecution = false;

    switch (event.dwDebugEventCode)
    {
        case EXCEPTION_DEBUG_EVENT:
            stopExecution = handleException(event.u.Exception.ExceptionRecord, dwContinueStatus);
            break;
        /*/case CREATE_PROCESS_DEBUG_EVENT:
            //std::cout << "Create_Process_Debug_Event\n";
            break;
        case CREATE_THREAD_DEBUG_EVENT:
            //std::cout << "CREATE_THREAD_DEBUG_EVENT\n";
            break;
        case EXIT_PROCESS_DEBUG_EVENT:
            //std::cout << "EXIT_PROCESS_DEBUG_EVENT\n";
            break;
        case EXIT_THREAD_DEBUG_EVENT:
            //std::cout << "EXIT_THREAD_DEBUG_EVENT\n";
            break;
        case LOAD_DLL_DEBUG_EVENT:
            //std::cout << "LOAD_DLL_DEBUG_EVENT\n";
            break;
        case UNLOAD_DLL_DEBUG_EVENT:
            //std::cout << "UNLOAD_DLL_DEBUG_EVENT\n";
            break;
        case OUTPUT_DEBUG_STRING_EVENT:
            //std::cout << "OUTPUT_DEBUG_STRING_EVENT\n";
            break;
        case RIP_EVENT:
            //std::cout << "RIP_EVENT\n";
            break;
        */default:
            break;
    }
    return stopExecution;
}

void printUsage()
{
    std::cout << "Usage: fuzzMonitor -targetProgram <targetApp> [-input directInput] [-corpusDir pathToCorpus]" << "\n";
}

void getCmdLine(int* numCmdLineArgs)
{
    cmdLineArgs = CommandLineToArgvW(GetCommandLine(), numCmdLineArgs);
    bool takeTargetName = false;
    bool takeInputString = false;
    bool takeCorpusDirPath = false;

    if (!cmdLineArgs)
    {
        std::cout << "Failed to get command line: " << GetLastError() << "\n";
        exit(1);
    }

    if (*numCmdLineArgs == 1)
    {
        printUsage();
        exit(0);
    }

    std::wcout << *numCmdLineArgs << "\n";

    for (size_t i = 0; i < *numCmdLineArgs; i++)
    {
        wchar_t* argName = NULL;


        if (takeTargetName)
        {
            takeTargetName = false;
            targetProgram = cmdLineArgs[i];
        }
        else if (takeInputString)
        {
            takeInputString = false;
            inputStringsFile = cmdLineArgs[i];
        }
        else if (takeCorpusDirPath)
        {
            takeCorpusDirPath = false;
            corpusDirectory = cmdLineArgs[i];
        }
        
        if (!wcscmp(cmdLineArgs[i], L"-targetProgram"))
        {
            takeTargetName = true;
        }
        else if (!wcscmp(cmdLineArgs[i], L"-input"))
        {
            takeInputString = true;
        }
        else if (!wcscmp(cmdLineArgs[i], L"-corpusDir"))
        {
            takeCorpusDirPath = true;
        }
    }
}

//It is possible to fuzz in two ways:
//  1. Passing the input to the target through command line: in this case, specify a file containing the inputs to use for fuzzing, one per line
//  2. Passing a file that the target will parse: in this case, specify a directory containing the corpus

int main(int argc, wchar_t** argv)
{
    DWORD dwContinueStatus;
    HANDLE crashFile = NULL;
    int numCmdLineArgs;
    TCHAR szDir[MAX_PATH];

    getCmdLine(&numCmdLineArgs);


    if (corpusDirectory != NULL)                                //the user decided to fuzz passing file inputs that the target will parse
    {
        size_t count = 0;
        StringCchCopy(szDir, MAX_PATH, corpusDirectory);
        StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

        HANDLE hFind = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA ffd;

        hFind = FindFirstFile(szDir, &ffd);

        if (!hFind)
        {
            std::cout << "error while looking for corpus\n";
            exit(1);
        }

        wchar_t targetCommandLine[500];
        wcscpy(targetCommandLine, L".\\");
        wcscpy(targetCommandLine, targetProgram);
        wcscat(targetCommandLine, L" .\\");
        wcscat(targetCommandLine, corpusDirectory);
        wcscat(targetCommandLine, L"\\");

        FindNextFile(hFind, &ffd);
        FindNextFile(hFind, &ffd);

        //============================================ FUZZING CYCLE ===================================================================================
        do
        {
            wchar_t command[500];
            wcscpy(command, targetCommandLine);

            wcscat(command, ffd.cFileName);

            std::wcout << command << "\n";

            if (!startAndDebugProcess(command))
            {
                std::cout << "Failed to create process: " << GetLastError() << "\n";
                exit(1);
            }

            while (WaitForDebugEvent(&event, INFINITE) == TRUE)
            {
                if (event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
                {
                    if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, dwContinueStatus))
                        std::cout << "Error while resuming execution: " << GetLastError();
                    goto out;
                }

                if (dispatchDebugEvent(&dwContinueStatus) == TRUE)
                {
                    crashFile = CreateFile(L"Crashes", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                    if (corpusDirectory != NULL)
                    {
                        WriteFile(crashFile, targetCommandLine, wcslen(targetCommandLine), NULL, NULL);
                    }
                    else if (inputStringsFile != NULL)
                    {
                        WriteFile(crashFile, inputStringsFile, wcslen(inputStringsFile), NULL, NULL);
                    }
                }

                if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, dwContinueStatus))
                    std::cout << "Error while resuming execution: " << GetLastError();
            }
            out:
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

        } while (FindNextFile(hFind, &ffd) != 0);       //this is to be done before starting to fuzz
        //============================================ FUZZING CYCLE ==========================================================================================================
            
    }

        return 0;
}