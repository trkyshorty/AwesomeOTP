#include <iostream>
#include <Windows.h>
#include "HardwareInformation.h"
#include <string>
#include <sstream>
#include <iomanip>

typedef int(__stdcall* GenerateOTP)(int, LPCWSTR, LPCWSTR, int*);

HardwareInformation m_HardwareInformation;

std::wstring GetHardwareID()
{
    std::wstring szOTPHardwareID = L"";

    for (size_t i = 0; i < m_HardwareInformation.Disk.size(); i++)
    {
        HardwareInformation::DiskObject& Disk{ m_HardwareInformation.Disk.at(i) };

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
            std::wstring szSignature = sstream.str();

            szOTPHardwareID = szSignature.substr(0, 4) + szPNPDeviceID.substr(szPNPDeviceID.length() - 16);
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
        m_HardwareInformation.LoadHardwareInformation();

        std::cout << "1) Hardware Information" << std::endl;
        std::cout << "2) Get OTP Hardware ID" << std::endl;
        std::cout << "3) Get OTP Code" << std::endl;
        std::cout << "Choose Option : ";

        std::wstring szInput;
        std::getline(std::wcin, szInput);

        int iOption = std::stoi(szInput);

        switch (iOption)
        {
            case 1:
            {
                for (size_t i = 0; i < m_HardwareInformation.Disk.size(); i++)
                {
                    HardwareInformation::DiskObject& Disk{ m_HardwareInformation.Disk.at(i) };

                    if (i == 0)
                    {
                        std::cout << "----------------------------" << std::endl;
                    }

                    std::wcout << "Name: " << Disk.Name << std::endl;
                    std::wcout << "Model: " << Disk.Model << std::endl;
                    std::wcout << "Serial Number: " << Disk.SerialNumber << std::endl;
                    std::wcout << "Size: " << Disk.Size << " GB" << std::endl;

                    std::wcout << "PNP Device ID: " << Disk.PNPDeviceID << std::endl;
                    std::wcout << "Signature: " << Disk.Signature << std::endl;

                    std::wstring szIsBootDevice = Disk.IsBootDrive ? L"TRUE" : L"FALSE";
                    std::wcout << "Boot Drive: " << szIsBootDevice << std::endl;

                    std::wstring szPNPDeviceID = Disk.PNPDeviceID;

                    if (Disk.Signature == 0)
                    {
                        std::wcout << "OTP Hardware ID: " << L"0000" + szPNPDeviceID.substr(szPNPDeviceID.length() - 16) << std::endl;
                    }
                    else
                    {
                        std::wstringstream sstream;
                        sstream << std::hex << Disk.Signature;
                        std::wstring szSignature = sstream.str();

                        std::wcout << "OTP Hardware ID: " << szSignature.substr(0, 4) + szPNPDeviceID.substr(szPNPDeviceID.length() - 16) << std::endl;
                    }

                    std::cout << "----------------------------" << std::endl;
                }
            }
            break;

            case 2:
            {
                std::wcout << L"OTP Hardware ID : " << GetHardwareID() << std::endl;
            }
            break;

            case 3:
            {
                std::cout << "Enter OTP Password : ";

                std::wstring szOTPPassword;
                std::getline(std::wcin, szOTPPassword);

                std::wstring szOTPHardwareID = GetHardwareID();

                auto pGenerateOTPAddress = (LPVOID)((DWORD)(hModule)+0x6327);
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
            break;

            default:
                std::cout << "Invalid option selected" << std::endl;
                break;
        }
    }
    else
    {
        std::cout << "Library Not Loaded!" << std::endl;
    }

    while (true)
    {
        Sleep(1000);
    }

    return 0;
}