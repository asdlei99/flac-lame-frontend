//
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Date.h"
#include "Hash.h"
#include "String.h"

namespace File
{
	inline bool IsDir(const wchar_t *path)  { return (::GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	inline bool Exists(const wchar_t *path) { return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES; }
	Date        LastModified(const wchar_t *path);
	Date        Created(const wchar_t *path);
	bool        WriteUtf8(const wchar_t *path, const wchar_t *data, String *pErr=0);

	struct Access { enum Type { READONLY, READWRITE }; };

	class Raw { // automation to a HANDLE of a file
	public:
		Raw() : _hFile(0) { }
		~Raw() { close(); }

		HANDLE hFile() const { return _hFile; }
		void   close()       { if(_hFile) { ::CloseHandle(_hFile); _hFile = 0; } }
		int    size()        { return ::GetFileSize(_hFile, 0); }
		bool   open(const wchar_t *path, Access::Type access, String *pErr=0);
		bool   setNewSize(int newSize, String *pErr=0);
		bool   truncate(String *pErr=0) { return setNewSize(0, pErr); }
		bool   getContent(Array<BYTE> *pBuf, String *pErr=0);
		bool   write(const Array<BYTE> *pData, String *pErr=0) { return write(&(*pData)[0], pData->size(), pErr); }
		bool   write(const BYTE *pData, int sz, String *pErr=0);
		bool   rewind(String *pErr=0);
	private:
		HANDLE _hFile;
	};

	class Mapped { // automation to a memory-mapped file
	public:
		Mapped() : _hMap(0), _pMem(0), _size(0) { }
		~Mapped() { close(); }

		void  close();
		bool  open(const wchar_t *path, Access::Type access, String *pErr=0);
		int   size() const     { return _size; }
		BYTE* pMem() const     { return (BYTE*)_pMem; }
		BYTE* pPastMem() const { return pMem() + size(); }
		bool  setNewSize(int newSize, String *pErr=0);
		bool  getContent(Array<BYTE> *pBuf, int offset=0, int numBytes=-1, String *pErr=0);
		bool  getContent(String *pBuf, int offset=0, int numChars=-1, String *pErr=0);
	private:
		Raw    _file;
		HANDLE _hMap;
		void  *_pMem;
		int    _size;
	};

	class Text { // automation to text files
	public:
		Text() : _p(0), _idxLine(-1) { }
		
		bool          load(const wchar_t *path, String *pErr=0);
		bool          load(const Mapped *pfm);
		bool          nextLine(String *pBuf);
		int           curLineIndex() const { return _idxLine; }
		void          rewind()             { _p = _text.ptrAt(0); _idxLine = -1; }
		const String* text() const         { return &_text; }
	private:
		String   _text;
		wchar_t *_p;
		int      _idxLine;
	};

	class Ini { // automation to INI files, loading into a hash
	public:
		Hash<Hash<String>> sections;
		Hash<String>& operator[](const wchar_t *key) { return sections[key]; }
		Hash<String>& operator[](const String& key)  { return sections[key]; }

		Ini& setPath(const wchar_t *iniPath) { _path = iniPath; return *this; }
		bool load(String *pErr=0);
		bool serialize(String *pErr=0) const;
	private:
		String _path;
		int _countSections(File::Text *fin) const;
	};

	class Enum { // automation for directory enumeration
	public:
		explicit Enum(const wchar_t *pattern);
		Enum(const wchar_t *path, const wchar_t *pattern);
		~Enum();

		wchar_t* next(wchar_t *buf);
	private:
		HANDLE          _hFind;
		WIN32_FIND_DATA _wfd;
		wchar_t        *_pattern;
	};
}