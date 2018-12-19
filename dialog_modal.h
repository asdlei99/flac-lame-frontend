/**
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * Copyright 2017-present Rodrigo Cesar de Freitas Dias
 * This library is released under the MIT License
 */

#pragma once
#include "internals/base_dialog.h"
#include "internals/base_msg_impl.h"
#include "internals/base_text_impl.h"
#include "internals/base_thread_impl.h"
#include "internals/styler.h"
#include "wnd.h"

namespace wl {

// Inherit from this class to have a modal dialog popup.
class dialog_modal :
	public wnd,
	public wli::base_msg_impl<INT_PTR>,
	public wli::base_thread_impl<INT_PTR, TRUE>,
	public wli::base_text_impl<dialog_modal>
{
private:
	HWND                            _hWnd = nullptr;
	wli::base_msg<INT_PTR>          _baseMsg{_hWnd};
	wli::base_thread<INT_PTR, TRUE> _baseThread{_baseMsg};
	wli::base_dialog                _baseDialog{_hWnd, _baseMsg};

public:
	// Defines window creation parameters.
	wli::base_dialog::setup_vars setup;

	// Wraps window style changes done by Get/SetWindowLongPtr.
	wli::styler<dialog_modal> style{this};

protected:
	dialog_modal() :
		wnd(_hWnd), base_msg_impl(_baseMsg), base_thread_impl(_baseThread), base_text_impl(_hWnd)
	{
		this->base_msg_impl::on_message(WM_CLOSE, [this](params) noexcept->INT_PTR {
			EndDialog(this->_hWnd, IDOK);
			return TRUE;
		});
	}

public:
	dialog_modal(dialog_modal&&) = default;
	dialog_modal& operator=(dialog_modal&&) = default; // movable only

	// Shows the modal dialog, returning only after the modal is closed.
	int show(HWND hParent) {
		INT_PTR ret = this->_baseDialog.dialog_box_param(this->setup, hParent);

		if (!ret) {
			throw std::invalid_argument("DialogBoxParam failed for modal dialog, invalid parent.");
		} else if (ret == -1) {
			throw std::system_error(GetLastError(), std::system_category(),
				"DialogBoxParam failed for modal dialog");
		}
		return static_cast<int>(ret); // value passed to EndDialog()
	}

	// Shows the modal dialog, returning only after the modal is closed.
	int show(const wnd* parent) {
		return this->show(parent->hwnd());
	}

protected:
	// Centers the modal dialog onto its parent.
	void center_on_parent() const noexcept {
		RECT rc{}, rcParent{};
		GetWindowRect(this->_hWnd, &rc);
		GetWindowRect(GetParent(this->_hWnd), &rcParent); // both relative to screen
		SetWindowPos(this->_hWnd, nullptr,
			rcParent.left + (rcParent.right - rcParent.left)/2 - (rc.right - rc.left)/2,
			rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rc.bottom - rc.top)/2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
};

}//namespace wl
