//
// Ordinary string implementation, ever-growing, with buffer capabilities.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Array.h"
#include <Windows.h>

class String {
public:
	String()                    { }
	String(const wchar_t *s)    { operator=(s); }
	String(const String& other) { operator=(other); }

	int            len() const                            { return _arr.size() ? ::lstrlen(&_arr[0]) : 0; }
	const wchar_t* str() const                            { return _arr.size() ? &_arr[0] : L""; }
	bool           isEmpty() const                        { return !_arr.size() || _arr[0] == L'\0'; }
	const wchar_t& operator[](int index) const            { return _arr[index]; }
	wchar_t&       operator[](int index)                  { return _arr[index]; }
	const wchar_t* ptrAt(int index) const                 { return &_arr[index]; }
	wchar_t*       ptrAt(int index)                       { return &_arr[index]; }
	String&        operator=(const String& other)         { return operator=(other.str()); }
	String&        operator=(const wchar_t *s);
	bool           equalSens(const wchar_t *s) const      { return !::lstrcmp(str(), s); }
	bool           equalInsens(const wchar_t *s) const    { return !::lstrcmpi(str(), s); }
	bool           equalSens(const String& other) const   { return equalSens(other.str()); }
	bool           equalInsens(const String& other) const { return equalInsens(other.str()); }
	String&        reserve(int numCharsWithoutNull);
	int            reserved() const                       { return _arr.size() - 1; }
	String&        append(const String& other)            { return append(other.str()); }
	String&        append(const wchar_t *s);
	String&        append(wchar_t ch);
	String&        insert(int at, const wchar_t *s);
	String&        fmtv(const wchar_t *fmt, va_list args, int at=0);
	String&        fmt(const wchar_t *fmt, ...);
	String&        appendfmt(const wchar_t *fmt, ...);
	String&        copyFrom(const wchar_t *src, int numChars);
	String&        appendFrom(const wchar_t *src, int numChars);
	String&        trim();
	String&        getSubstr(const wchar_t *src, int start, int length=0);
	bool           startsWith(const wchar_t *s) const;
	bool           endsWith(const wchar_t *s) const;
	bool           endsWith(wchar_t ch) const;
	bool           isInt() const;
	bool           isFloat() const;
	int            toInt() const                          { return this->isInt() ? ::_wtoi(this->str()) : 0; }
	double         toFloat() const                        { return this->isFloat() ? ::_wtof(this->str()) : 0; }
	int            find(wchar_t ch) const;
	int            find(const wchar_t *substring) const;
	int            findr(wchar_t ch) const;
	String&        replace(const wchar_t *target, const wchar_t *replacement);
	String&        invert();
	void           explode(const wchar_t *delimiters, Array<String> *pBuf) const;
	String&        fromUtf8(const BYTE *data, int length);

	static int __cdecl Sort(const void *a, const void *b);

private:
	Array<wchar_t> _arr;
};