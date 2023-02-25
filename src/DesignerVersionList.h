#pragma once

#include <string>
#include <vector>
#include <map>


enum BuildType
{
	/*
	 * Byte format:   msb [TT NNNNNN] lsb
	 *
	 * where T = build type and N = build number
	 */

	BuildType_Release = 0,
	BuildType_ReleaseCandidate = 0x40,
	BuildType_Beta = 0x80,
	BuildType_Debug = 0xc0,

	BuildType_Mask = 0xc0,
	BuildNumber_Mask = 0x3f
};

class DesignerVersion
{
public:
	DesignerVersion(std::wstring path = std::wstring(), int major = 0, int minor = 0, int patch = 0, int build = 0) : 
		m_path(path), m_Major(major), m_Minor(minor), m_Patch(patch), m_Build(build) {};
	
	const bool operator <(const DesignerVersion& rhs) const;
	
	bool isValid() const { return m_Major != 0 && m_Minor != 0; }

	std::wstring toString() const;

	void setPath(const std::wstring& path) { m_path = path; }
	std::wstring path() const { return m_path; }
private:
	int m_Major = 0;
	int m_Minor = 0;
	int m_Patch = 0;
	int m_Build = 0;
	std::wstring m_path;
};

class DesignerVersionList : public std::vector<DesignerVersion>
{
public:
	DesignerVersionList();
	void doSearch();
private:
	void searchVersions(std::wstring path, int depth = 0);
	void getVersionsFromPaths();
	DesignerVersion guessDesignerVersionFromCreatedDate(const std::wstring& path);
	std::vector<std::wstring> m_paths;
	static std::map<uint64_t, DesignerVersion> m_versionMap;
};