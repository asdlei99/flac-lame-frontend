/*!
 * File handling.
 * Part of OWL - Object Win32 Library.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <unordered_map>
#include "System.h"

namespace owl {

struct File final {
	static inline bool  Exists(const wchar_t *path)                { return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES; }
	static inline bool  Exists(const std::wstring& path)           { return Exists(path.c_str()); }
	static inline bool  IsDir(const wchar_t *path)                 { return (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	static inline bool  IsDir(const std::wstring& path)            { return IsDir(path.c_str()); }
	static bool         Delete(const wchar_t *path, std::wstring *pErr=nullptr);
	static inline bool  Delete(const std::wstring& path, std::wstring *pErr=nullptr) { return Delete(path.c_str(), pErr); }
	static bool         CreateDir(const wchar_t *path);
	static inline bool  CreateDir(const std::wstring& path)        { return CreateDir(path.c_str()); }
	static Date         DateLastModified(const wchar_t *path);
	static inline Date  DateLastModified(const std::wstring& path) { return DateLastModified(path.c_str()); }
	static Date         DateCreated(const wchar_t *path);
	static inline Date  DateCreated(const std::wstring& path)      { return DateCreated(path.c_str()); }
	static bool         WriteUtf8(const wchar_t *path, const wchar_t *data, std::wstring *pErr=nullptr);
	static bool         Unzip(const wchar_t *zip, const wchar_t *destFolder, std::wstring *pErr=nullptr);
	static inline bool  Unzip(const std::wstring& zip, const std::wstring& destFolder, std::wstring *pErr=nullptr) { return Unzip(zip.c_str(), destFolder.c_str(), pErr); }
	static int          IndexOfBin(const BYTE *pData, size_t dataLen, const wchar_t *what, bool asWideChar);

	// Path string utilities.
	struct Path final {
		static void                ChangeExtension(std::wstring& path, const wchar_t *extWithoutDot);
		static void                TrimBackslash(std::wstring& path);
		static std::wstring        GetPath(const wchar_t *path);
		inline static std::wstring GetPath(const std::wstring& path) { return GetPath(path.c_str()); }
		static std::wstring        GetFilename(const std::wstring& path);
	};

	enum class Access { READONLY, READWRITE };

	// Automation to a HANDLE of a file.
	class Raw final {
	private:
		HANDLE _hFile;
		Access _access;
	public:
		Raw()  : _hFile(nullptr), _access(Access::READONLY) { }
		~Raw() { close(); }

		HANDLE hFile() const     { return _hFile; }
		Access getAccess() const { return _access; }
		void   close();
		size_t size() const      { return ::GetFileSize(_hFile, nullptr); }
		bool   open(const wchar_t *path, Access access, std::wstring *pErr=nullptr);
		bool   open(const std::wstring& path, Access access, std::wstring *pErr=nullptr) { return open(path.c_str(), access, pErr); }
		bool   setNewSize(size_t newSize, std::wstring *pErr=nullptr);
		bool   truncate(std::wstring *pErr=nullptr)                                      { return setNewSize(0, pErr); }
		bool   getContent(std::vector<BYTE>& buf, std::wstring *pErr=nullptr) const;
		bool   write(const BYTE *pData, size_t sz, std::wstring *pErr=nullptr);
		bool   write(const std::vector<BYTE>& data, std::wstring *pErr=nullptr)          { return write(&data[0], data.size(), pErr); }
		bool   rewind(std::wstring *pErr=nullptr);
	};

	// Automation to a memory-mapped file.
	class Mapped final {
	private:
		Raw    _file;
		HANDLE _hMap;
		void  *_pMem;
		size_t _size;
	public:
		Mapped() : _hMap(nullptr), _pMem(nullptr), _size(0) { }
		~Mapped() { close(); }

		Access getAccess() const { return _file.getAccess(); }
		void   close();
		bool   open(const wchar_t *path, Access access, std::wstring *pErr=nullptr);
		bool   open(const std::wstring& path, Access access, std::wstring *pErr=nullptr) { return open(path.c_str(), access, pErr); }
		size_t size() const     { return _size; }
		BYTE*  pMem() const     { return (BYTE*)_pMem; }
		BYTE*  pPastMem() const { return pMem() + size(); }
		bool   setNewSize(size_t newSize, std::wstring *pErr=nullptr);
		bool   getContent(std::vector<BYTE>& buf, int offset=0, int numBytes=-1, std::wstring *pErr=nullptr) const;
		bool   getContent(std::wstring& buf, int offset=0, int numChars=-1, std::wstring *pErr=nullptr) const;
	};

	// Automation to read text files line-by-line.
	class Text final {
	private:
		std::wstring _text;
		wchar_t     *_p;
		int          _idxLine;
	public:
		Text() : _p(nullptr), _idxLine(-1) { }

		bool load(const wchar_t *path, std::wstring *pErr=nullptr);
		bool load(const std::wstring& path, std::wstring *pErr=nullptr) { return load(path.c_str(), pErr); }
		bool load(const Mapped& fm);
		bool nextLine(std::wstring& buf);
		int  curLineIndex() const        { return _idxLine; }
		void rewind()                    { _p = &_text[0]; _idxLine = -1; }
		const std::wstring& text() const { return _text; }
	};

	// Automation to INI files, loaded into two nested associative arrays.
	class Ini final {
	private:
		std::wstring _path;
	public:
		std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::wstring>> sections;
		Ini& setPath(const wchar_t *iniPath)      { _path = iniPath; return *this; }
		Ini& setPath(const std::wstring& iniPath) { return setPath(iniPath.c_str()); }
		bool load(std::wstring *pErr=nullptr);
		bool serialize(std::wstring *pErr=nullptr) const;
	private:
		int _countSections(File::Text *fin) const;
	};

	// Automation for directory enumeration.
	class Listing final {
	private:
		HANDLE          _hFind;
		WIN32_FIND_DATA _wfd;
		std::wstring    _pattern;
	public:
		explicit Listing(const wchar_t *pattern);
		Listing(const wchar_t *path, const wchar_t *pattern);
		Listing(const std::wstring& path, const wchar_t *pattern) : Listing(path.c_str(), pattern) { }
		~Listing();
		bool next(std::wstring& buf);
		
		static std::vector<std::wstring> GetAll(const wchar_t *pattern);
		static std::vector<std::wstring> GetAll(const wchar_t *path, const wchar_t *pattern);
		static std::vector<std::wstring> GetAll(const std::wstring& path, const wchar_t *pattern) { return GetAll(path.c_str(), pattern); }
	};
};

}//namespace owl