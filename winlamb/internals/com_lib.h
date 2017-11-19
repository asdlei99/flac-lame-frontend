/**
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * Copyright 2017-present Rodrigo Cesar de Freitas Dias
 * This library is released under the MIT License
 */

#pragma once
#include <Windows.h>
#include <objbase.h>

namespace wl {
namespace wli {

// Smart class to automate CoInitialize calls and cleanup.
class com_lib {
private:
	HRESULT _hr = -1;

public:
	enum class init { NOW, LATER };

	~com_lib() {
		// https://stackoverflow.com/q/47123650/6923555
		if (SUCCEEDED(this->_hr)) {
			CoUninitialize();
		}
	}

	explicit com_lib(init when) {
		if (when == init::NOW) {
			this->initialize();
		}
	}

	void initialize() {
		if (FAILED(this->_hr)) { // so that initialize() can be called multiple times
			this->_hr = CoInitialize(nullptr);
		}
	}

	HRESULT hresult() const {
		return this->_hr;
	}
};

}//namespace wli
}//namespace wl
