/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_dialog.h"
#include "base_loop.h"
#include "base_text.h"

/**
 *                              +--------------------- msgs_[any] <----------------------------+
 *             +-- base_msgs <--+                                                              |
 *             |                +-- base_threaded <--+                                         +-- [user]
 * base_wnd <--+                                     +-- base_dialog <--+                      |
 *             +------------ base_wheel <------------+                  +-- dialog_modeless <--+
 *             |                                                        |
 *             +--------------------- base_text <-----------------------+
 */

namespace wl {

// Inherit from this class to have a dialog modeless popup.
class dialog_modeless :
	public    base::dialog,
	protected base::text<dialog_modeless>
{
private:
	base::loop* _parent;
protected:
	base::dialog::setup_vars setup;

	dialog_modeless(size_t msgsReserve = 0) : dialog(msgsReserve + 1) {
		this->on_message(WM_CLOSE, [&](const params&)->INT_PTR {
			DestroyWindow(this->hwnd());
			return TRUE;
		});
		this->on_message(WM_NCDESTROY, [&](const params&)->INT_PTR {
			this->_parent->_remove_modeless(this->hwnd());
			return TRUE;
		});
	}

public:
	void show(base::loop* parent) {
		if (!this->dialog::_basic_initial_checks(this->setup)) return;
		this->_parent = parent;
		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));

		if (!CreateDialogParamW(hInst, MAKEINTRESOURCE(this->setup.dialogId),
			parent->hwnd(), base::dialog::_dialog_proc,
			reinterpret_cast<LPARAM>(this)) )
		{
			OutputDebugStringW(L"ERROR: modeless dialog not created, CreateDialogParam failed.\n");
			return;
		}

		parent->_add_modeless(this->hwnd());
		ShowWindow(this->hwnd(), SW_SHOW);
	}

	void show(base::loop* parent, POINT clientPos) {
		this->show(parent);
		POINT parentPos = clientPos;
		ClientToScreen(parent->hwnd(), &parentPos); // now relative to parent
		SetWindowPos(this->hwnd(), nullptr,
			parentPos.x, parentPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
};

}//namespace wl