/**
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * Copyright 2017-present Rodrigo Cesar de Freitas Dias
 * This library is released under the MIT License
 */

#pragma once
#include <Windows.h>

namespace wl {
namespace wli {

template<typename derivedT, typename baseT>
class w_focus : public baseT {
protected:
	w_focus() = default;

public:
	// Simple wrapper to SetFocus.
	derivedT& set_focus() noexcept {
		SetFocus(this->hwnd());
		return *static_cast<derivedT*>(this);
	}
};

}//namespace wli
}//namespace wl
