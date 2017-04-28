#include "stdafx.h"

#include "FileSystemHelper.h"
#include "Logger.h"

using namespace std;
using std::function;

list<wstring> FileSystemHelper::getAllSubDirectories(const wstring& directoryPath)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	list<wstring> subDirectories = list<wstring>();

	log().debug(__FUNCTION__, L"Getting Sub-Directories of " + directoryPath);

	if (directoryPath.length() > MAX_PATH - 3) {
		log().error(__FUNCTION__, directoryPath + L" length is greater than my max path allowed");

		throw FileSystemHelperException();
	}

	/* Prepare string for use with FindFile functions.  First, copy the
	   string to a buffer, then append '\*' to the directory name. */

	hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		log().error(__FUNCTION__, L"Failed to get first file in " + directoryPath + L"Error: " + to_wstring(GetLastError()));

		throw FileSystemHelperException();
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!wcscmp(ffd.cFileName, L".") || (!wcscmp(ffd.cFileName, L".."))) {
				continue;
			}
			wstring subDir = wstring(directoryPath + L"\\" + ffd.cFileName);
			list<wstring> subDirList = list<wstring>();

			log().debug(__FUNCTION__, L"Found sub directory " + subDir + L" in " + directoryPath);
			
			subDirectories.push_back(subDir);
			subDirList = getAllSubDirectories(subDir);
			subDirectories.insert(subDirectories.end(), subDirList.begin(), subDirList.end());
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		log().error(__FUNCTION__, L"Failed to get new file in " + directoryPath + L"Error: " + to_wstring(GetLastError()));

		FindClose(hFind);

		throw FileSystemHelperException();
	}

	FindClose(hFind);
	return subDirectories;
}

list<wstring> FileSystemHelper::getAllFilesInDir(const wstring& directoryPath)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	list<wstring> fileNames = list<wstring>();

	log().debug(__FUNCTION__, L"Getting file names in " + directoryPath);

	if (directoryPath.length() > MAX_PATH - 3) {
		log().error(__FUNCTION__, directoryPath + L" length is greater than my max path allowed");

		throw FileSystemHelperException();
	}

	/* Prepare string for use with FindFile functions.  First, copy the
	string to a buffer, then append '\*' to the directory name. */

	hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		log().error(__FUNCTION__, L"Failed to get first file in " + directoryPath + L"Error: " + to_wstring(GetLastError()));

		throw FileSystemHelperException();
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else {
			fileNames.push_back(ffd.cFileName);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		log().error(__FUNCTION__, L"Failed to get new file in " + directoryPath + L"Error: " + to_wstring(GetLastError()));

		FindClose(hFind);

		throw FileSystemHelperException();
	}

	FindClose(hFind);
	return fileNames;
}

bool compareFilesByAccessTime(const WIN32_FIND_DATA & a, const WIN32_FIND_DATA & b)
{
	return (CompareFileTime(&a.ftLastAccessTime, &b.ftLastAccessTime) < 0) ? true : false;
}

bool compareFilesLexicographic(const WIN32_FIND_DATA & a, const WIN32_FIND_DATA & b)
{
	return a.cFileName < b.cFileName;
}

bool compareFilesByWriteTime(const WIN32_FIND_DATA & a, const WIN32_FIND_DATA & b)
{
	return (CompareFileTime(&a.ftLastWriteTime, &b.ftLastWriteTime) < 0) ? true : false;
}

void FileSystemHelper::getFirstAndLastByCoparable(const wstring & directoryPath, wstring & firstFile, wstring & lastFile, std::function<bool(const WIN32_FIND_DATA &, const WIN32_FIND_DATA &)> compare)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	list<WIN32_FIND_DATA> filesData = list<WIN32_FIND_DATA>();

	log().debug(__FUNCTION__, L"Getting first and last file name in " + directoryPath + L" according to access time order");

	if (directoryPath.length() > MAX_PATH - 3) {
		log().error(__FUNCTION__, directoryPath + L" length is greater than my max path allowed");

		throw FileSystemHelperException();
	}

	/* Prepare string for use with FindFile functions.  First, copy the
	string to a buffer, then append '\*' to the directory name. */

	hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		log().error(__FUNCTION__, L"Failed to get first file in " + directoryPath + L"Error: " + to_wstring(GetLastError()));

		throw FileSystemHelperException();
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else {
			filesData.push_back(ffd);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		log().error(__FUNCTION__, L"Failed to get new file in " + directoryPath + L"Error: " + to_wstring(GetLastError()));

		FindClose(hFind);

		throw FileSystemHelperException();
	}

	FindClose(hFind);

	filesData.sort(compare);

	firstFile = filesData.front().cFileName;
	lastFile = filesData.back().cFileName;
}


void FileSystemHelper::getFirstAndLastAccessTimeOrder(const wstring& directoryPath, wstring& firstFile, wstring& lastFile)
{
	log().debug(__FUNCTION__, L"Getting first and last file name in " + directoryPath + L" according to access time order");

	getFirstAndLastByCoparable(directoryPath, firstFile, lastFile, compareFilesByAccessTime);
}

void FileSystemHelper::getFirstAndLastFileLexicographicOrder(const wstring& directoryPath, wstring& firstFile, wstring& lastFile)
{
	log().debug(__FUNCTION__, L"Getting first and last file name in " + directoryPath + L" according to lexicographic order");

	getFirstAndLastByCoparable(directoryPath, firstFile, lastFile, compareFilesLexicographic);
}

void FileSystemHelper::getFirstAndLastFileWriteTimeOrder(const wstring& directoryPath, wstring& firstFile, wstring& lastFile)
{
	log().debug(__FUNCTION__, L"Getting first and last file name in " + directoryPath + L" according to write time order");

	getFirstAndLastByCoparable(directoryPath, firstFile, lastFile, compareFilesByWriteTime);
}