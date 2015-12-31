
#pragma once
#include "../wolf/wolf.h"

struct Convert final {
public:
	static bool pathsAreValid(const wolf::FileIni& ini, std::wstring *pErr=nullptr);

	static bool toWav(const wolf::FileIni& ini, std::wstring src, std::wstring dest, bool delSrc,
		std::wstring *pErr=nullptr);
	
	static bool toFlac(const wolf::FileIni& ini, std::wstring src, std::wstring dest, bool delSrc,
		const std::wstring& quality, std::wstring *pErr=nullptr);
	
	static bool toMp3(const wolf::FileIni& ini, std::wstring src, std::wstring dest, bool delSrc,
		const std::wstring& quality, bool isVbr, std::wstring *pErr=nullptr);
private:
	static void _changeExtension(std::wstring& filePath, const wchar_t *newExtensionWithoutDot);
	static bool _checkDestFolder(std::wstring& dest, std::wstring *pErr=nullptr);
	static bool _execute(const std::wstring& cmdLine, const std::wstring& src, bool delSrc,
		std::wstring *pErr=nullptr);
};