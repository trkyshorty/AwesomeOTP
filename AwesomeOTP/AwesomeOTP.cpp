#include <iostream>
#include <Windows.h>
#include "HardwareID.h"
#include <string>
#include <sstream>
#include <iomanip>

typedef int(__stdcall* GenerateOTP)(int, LPCWSTR, LPCWSTR, int*);

std::wstring GetHardwareID()
{
    HardwareID HWID{};

    std::wstring szOTPHardwareID = L"";

    for (size_t i = 0; i < HWID.Disk.size(); i++)
    {
        HardwareID::DiskObject& Disk{ HWID.Disk.at(i) };

        if (!Disk.IsBootDrive)
            continue;

        std::wstring szPNPDeviceID = Disk.PNPDeviceID;

        if (Disk.Signature == 0)
        {
            szOTPHardwareID = L"0000" + szPNPDeviceID.substr(szPNPDeviceID.length() - 16);
        }
        else
        {
            std::wstringstream sstream;
            sstream << std::hex << Disk.Signature;
            std::wstring hexStr = sstream.str();

            szOTPHardwareID = hexStr.substr(0, 4) + szPNPDeviceID.substr(szPNPDeviceID.length() - 16);
        }
    }

    return szOTPHardwareID;
}

int main()
{
    LPCWSTR libraryName = L"AnyOTPBiz.dll";

    HMODULE hModule = LoadLibraryExW(libraryName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (hModule != NULL)
    {
        std::cout << "Enter OTP Password : ";
        std::wstring szOTPPassword;
        std::getline(std::wcin, szOTPPassword);

        std::wstring szOTPHardwareID = GetHardwareID();

        auto pGenerateOTPAddress = (LPVOID)((DWORD)(hModule) + 0x6327);
        GenerateOTP pGenerateOTP = reinterpret_cast<GenerateOTP>(pGenerateOTPAddress);

        int iCodeAddress;
        pGenerateOTP(0, szOTPHardwareID.c_str(), szOTPPassword.c_str(), &iCodeAddress);

        const int bufferSize = 6 * 2;
        WCHAR buffer[bufferSize] = { 0 };

        SIZE_T bytesRead;
        if (ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(iCodeAddress), buffer, bufferSize, &bytesRead))
        {
            std::wcout << L"OTP Code : " << buffer << std::endl;
        }
        else
        {
            std::cerr << "ReadProcessMemory Failed" << std::endl;
        }
    }
    else
    {
        std::cout << "DLL Not Loaded!" << std::endl;
    }

    while (true)
    {
        Sleep(1000);
    }

    return 0;
}