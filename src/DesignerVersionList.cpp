#include "DesignerVersionList.h"

#include <windows.h>
#include <Shlobj.h>
#include <iostream>
#include <locale>
#include <codecvt>
#include <format>
#include <algorithm>
#include <iostream>


// Search to a depth of 3 because some folks make directory structures like Pharos Controls/Designer/2.9/Designer2, etc
#define MAX_DEPTH_TO_SEARCH 3

DesignerVersion DesignerVersionList::guessDesignerVersionFromCreatedDate(const std::wstring& path)
{
    auto fileHandle = 
        CreateFile(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return DesignerVersion(path);
    }

    FILETIME creationTime;
    if (!GetFileTime(fileHandle, &creationTime, NULL, NULL))
    {
        return DesignerVersion(path);
    }

    uint64_t fileTime = (uint64_t)creationTime.dwLowDateTime | (uint64_t)creationTime.dwHighDateTime << 32 ;

    switch(fileTime)
    {
    case 131230781260000000:
        return DesignerVersion(path, 2, 1, 5, 0);
    default:
        return DesignerVersion(path);
    }

}

const bool DesignerVersion::operator<(const DesignerVersion& rhs) const
{
    if (m_Major != rhs.m_Major)
    {
        return (m_Major < rhs.m_Major);
    }
    else if (m_Minor != rhs.m_Minor)
    {
        return (m_Minor < rhs.m_Minor);
    }
    else if (m_Patch != rhs.m_Patch)
    {
        return (m_Patch < rhs.m_Patch);
    }

    const uint8_t _buildType = m_Build & BuildType_Mask;
    const uint8_t rhsBuildType = rhs.m_Build & BuildType_Mask;
    if (_buildType != rhsBuildType)
    {
        return (_buildType > rhsBuildType); // build type hierarchy is in reverse order
    }

    const uint8_t _buildNum = m_Build & BuildNumber_Mask;
    const uint8_t rhsBuildNum = rhs.m_Build & BuildNumber_Mask;
    return _buildNum < rhsBuildNum;
}


std::wstring DesignerVersion::toString() const
{
    if (!isValid())
    {
        return m_path;
    }
    std::wstring versionString;
    versionString.append(std::to_wstring(m_Major));
    versionString.append(L".");
    versionString.append(std::to_wstring(m_Minor));
    versionString.append(L".");
    versionString.append(std::to_wstring(m_Patch));

    if ((m_Build & BuildType_Mask) == BuildType_Release)
    {
        return versionString;
    }

    versionString.append(L" ");

    switch (m_Build & BuildType_Mask)
    {
    case BuildType_ReleaseCandidate: versionString.append(L"RC"); break;
    case BuildType_Beta: versionString.append(L"BETA"); break;
    case BuildType_Debug: versionString.append(L"DEBUG"); break;
    default: break;
    }

    if (m_Build & BuildType_Mask)
    {
        versionString.append(std::to_wstring(m_Build & BuildNumber_Mask));
    }

    return versionString;
}

DesignerVersionList::DesignerVersionList()
{
    
}

void DesignerVersionList::doSearch()
{
    TCHAR pf[MAX_PATH];
    SHGetSpecialFolderPath(
        0,
        pf,
        CSIDL_PROGRAM_FILES,
        FALSE);

    std::wstring searchPath = std::wstring(pf) + std::wstring(L"\\Pharos Controls\\*");
    searchVersions(searchPath.c_str());

    SHGetSpecialFolderPath(
        0,
        pf,
        CSIDL_PROGRAM_FILESX86,
        FALSE);

    searchPath = std::wstring(pf) + std::wstring(L"\\Pharos Controls\\*");
    searchVersions(searchPath.c_str());

    getVersionsFromPaths();

    std::sort(this->begin(), this->end());
    std::reverse(this->begin(), this->end());
}

void DesignerVersionList::searchVersions(std::wstring path, int depth)
{
    // Recursively search for files called pharos_designer.exe
    std::wcout << L"Searching path " << path << L"\n";
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(path.c_str(), &data);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {

            if (data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
            {
                // Check the filename
                std::wstring filename(data.cFileName);
                if (filename == L"pharos_designer.exe")
                {
                    std::wstring targetPath = path.substr(0, path.length() - 2) + std::wstring(L"\\") + filename;
                    std::wcout << L"Adding " << targetPath << L"\n";
                    m_paths.push_back(targetPath);
                    break; // No need to dig further in this tree if we found one
                }
            }

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && 
                std::wstring(data.cFileName) != L"." &&
                std::wstring(data.cFileName) != L"..")
            {
                if (depth < MAX_DEPTH_TO_SEARCH)
                {
                    searchVersions(path.substr(0, path.length()-2) + std::wstring(L"\\") + 
                        std::wstring(data.cFileName) + std::wstring(L"\\*"),
                        depth + 1);
                }
            }

        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
}

void DesignerVersionList::getVersionsFromPaths()
{
    for (auto path : m_paths)
    {
        DWORD infoSize = GetFileVersionInfoSize(path.c_str(), 0);

        LPBYTE infoBuffer = new BYTE[infoSize];

        if (!GetFileVersionInfo(
            path.c_str(),
            0,
            infoSize,
            infoBuffer))
        {
            // Older versions of designer do not have embedded version info
            // Use the file creation date to guess the version
            push_back(guessDesignerVersionFromCreatedDate(path));
            continue;
        }

        UINT   size = 0;
        LPBYTE lpBuffer = NULL;

        if (VerQueryValue(infoBuffer, L"\\", (VOID FAR * FAR*) & lpBuffer, &size))
        {
            if (size)
            {
                VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                if (verInfo->dwSignature == 0xfeef04bd)
                {
                    DesignerVersion version(
                        path,
                        (verInfo->dwFileVersionMS >> 16) & 0xffff,
                        (verInfo->dwFileVersionMS >> 0) & 0xffff,
                        (verInfo->dwFileVersionLS >> 16) & 0xffff,
                        (verInfo->dwFileVersionLS >> 0) & 0xffff
                        );
                    push_back(version);
                    continue;
                }
            }
        }

        push_back(DesignerVersion(path));
    }
}
