#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "wbemuuid.lib")

using namespace std;

void get_CPU_INFO();
void get_GPU_INFO();
void get_DISK_INFO();
void get_MOTHERBOARD_INFO();
void get_RAM_INFO();
void get_STORAGE_INFO();

int main()
{

    cout << "Hello World!\n";
    get_CPU_INFO();
    get_GPU_INFO();
    get_DISK_INFO();
    get_MOTHERBOARD_INFO();
    get_RAM_INFO();
    get_STORAGE_INFO();
}

void get_CPU_INFO()
{
    SYSTEM_INFO sys_INFO;
    GetSystemInfo(&sys_INFO);

    printf("CPU Information: \n");
    printf("  Number of processors: %u\n", sys_INFO.dwNumberOfProcessors);
    printf("  Processor type: %u\n", sys_INFO.dwProcessorType);

    char CPUInfo[0x40] = { 0 };
    int CPURegs[4] = { -1 };

    __cpuidex(CPURegs, 0x80000002, 0);
    memcpy(CPUInfo, CPURegs, sizeof(CPURegs));

    __cpuidex(CPURegs, 0x80000003, 0);
    memcpy(CPUInfo + 16, CPURegs, sizeof(CPURegs));

    __cpuidex(CPURegs, 0x80000004, 0);
    memcpy(CPUInfo + 32, CPURegs, sizeof(CPURegs));

    printf("  CPU Brand: %s\n", CPUInfo);
}

void get_GPU_INFO()
{
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return;
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc);

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
            << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        cout << "Query for operating system name failed. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn == 0)
            break;

        VARIANT vtProp;

        hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        wcout << "GPU Name : " << vtProp.bstrVal << endl;
        VariantClear(&vtProp);

        hres = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
        wcout << "GPU RAM : " << vtProp.uintVal / (1024 * 1024) << " MB" << endl;
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}

void get_DISK_INFO()
{
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes))
    {
        printf("Disk Information: \n");
        printf("  Total Disk Space: %.2f GB\n", static_cast<double>(totalNumberOfBytes.QuadPart) / (1024 * 1024 * 1024));
        printf("  Free Disk Space: %.2f GB\n", static_cast<double>(totalNumberOfFreeBytes.QuadPart) / (1024 * 1024 * 1024));
        
    }
    else
    {
        printf("Error getting disk information.\n");
    }
}

void get_STORAGE_INFO()
{
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return;
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc);

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
            << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_DiskDrive"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        cout << "Query for disk drive information failed. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn == 0)
            break;

        VARIANT vtMediaType;
        VARIANT vtModel;

        hres = pclsObj->Get(L"MediaType", 0, &vtMediaType, 0, 0);
        hres = pclsObj->Get(L"Model", 0, &vtModel, 0, 0);

        wcout << L"Storage Information: \n";
        wcout << L"  Disk Type: " << vtMediaType.bstrVal << endl;
        wcout << L"  Disk Model: " << vtModel.bstrVal << endl;

        VariantClear(&vtMediaType);
        VariantClear(&vtModel);

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}

void get_MOTHERBOARD_INFO()
{
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return;
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc);

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
            << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_BaseBoard"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        cout << "Query for motherboard information failed. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    if (pEnumerator)
    {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn != 0)
        {
            VARIANT vtProp;

            hres = pclsObj->Get(L"Product", 0, &vtProp, 0, 0);
            wcout << "Motherboard Model: " << vtProp.bstrVal << endl;
            VariantClear(&vtProp);

            hres = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
            wcout << "Manufacturer: " << vtProp.bstrVal << endl;
            VariantClear(&vtProp);

            pclsObj->Release();
        }
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}

void get_RAM_INFO()
{
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    if (GlobalMemoryStatusEx(&memoryStatus))
    {
        printf("RAM Information: \n");
        printf("  Total Physical Memory: %.2f GB\n", static_cast<double>(memoryStatus.ullTotalPhys) / (1024 * 1024 * 1024));
        printf("  Available Physical Memory: %.2f GB\n", static_cast<double>(memoryStatus.ullAvailPhys) / (1024 * 1024 * 1024));
    }
    else
    {
        printf("Error getting RAM information.\n");
    }
}