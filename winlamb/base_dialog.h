/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_threaded.h"
#include "base_wheel.h"
#include "font.h"

/**
 *             +-- base_msgs <-- base_threaded <--+
 * base_wnd <--+                                  +-- base_dialog
 *             +---------- base_wheel <-----------+
 */

namespace wl {

class dialog_main;
class dialog_modal;
class dialog_modeless;
class dialog_control;

namespace base {

	class dialog :
		public    threaded<TRUE>,
		protected wheel
	{
	public:
		friend dialog_main;
		friend dialog_modal;
		friend dialog_modeless;
		friend dialog_control;

		struct setup_vars {
			int dialogId;
			setup_vars() : dialogId(0) { }
		};

		~dialog() {
			if (this->hwnd()) {
				SetWindowLongPtrW(this->hwnd(), GWLP_USERDATA, 0);
			}
		}

	protected:
		explicit dialog(size_t msgsReserve) : threaded(msgsReserve) {
			this->msgs::_defProc = [](const params&)->INT_PTR { // set default procedure
				return FALSE;
			};
		}

	private:
		bool _basic_initial_checks(const setup_vars& setup) const {
			if (this->hwnd()) {
				OutputDebugStringW(L"ERROR: tried to create dialog twice.\n");
				return false;
			}
			if (!setup.dialogId) {
				OutputDebugStringW(L"ERROR: dialog not created, no resource ID given.\n");
				return false;
			}
			return true;
		}

		static INT_PTR CALLBACK _dialog_proc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
			dialog* pSelf = nullptr;
			INT_PTR ret = FALSE;

			if (msg == WM_INITDIALOG) {
				pSelf = reinterpret_cast<dialog*>(lp);
				SetWindowLongPtrW(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
				font::set_ui_on_children(hDlg); // if user creates controls manually, font must be set manually on them
				pSelf->wnd::_hWnd = hDlg; // store HWND
			} else {
				pSelf = reinterpret_cast<dialog*>(GetWindowLongPtrW(hDlg, GWLP_USERDATA));
			}

			if (pSelf) {
				msgs::funcT* pFunc = pSelf->msgs::_msgInventory.find(msg);
				if (pFunc) ret = (*pFunc)(params{msg, wp, lp});
			}

			if (msg == WM_INITDIALOG) {
				pSelf->wheel::_apply_wheel_hover_behavior();
			} else if (msg == WM_NCDESTROY) { // cleanup
				SetWindowLongPtrW(hDlg, GWLP_USERDATA, 0);
				if (pSelf) {
					pSelf->wnd::_hWnd = nullptr;
				}
			}

			return ret;
		}
	};

}//namespace base
}//namespace wl