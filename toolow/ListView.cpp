
#include "ListView.h"
#include "Icon.h"

void ListView::Item::swapWith(int index)
{
	Item newItem = this->_list->items[index];

	int numCols = this->_list->columnCount();
	String oldTxt, newTxt;
	for(int c = 0; c < numCols; ++c) { // swap texts of all columns
		this->getText(&oldTxt, c); // get both texts
		newItem.getText(&newTxt, c);
		this->setText(newTxt.str(), c); // swap the texts
		newItem.setText(oldTxt.str(), c);
	}

	LPARAM oldp = this->getParam(); // swap LPARAMs
	this->setParam(newItem.getParam());
	newItem.setParam(oldp);

	int oldi = this->getIcon(); // swap icons
	this->setIcon(newItem.getIcon());
	newItem.setIcon(oldi);
}

ListView::Item& ListView::Item::ensureVisible()
{
	if(_list->getView() == View::VW_DETAILS) {
		// In details view, ListView_EnsureVisible() won't center the item vertically.
		// This new implementation has this behavior.
		RECT rc = { 0 };
		_list->getClientRect(&rc);
		int cyList = rc.bottom; // total height of list

		SecureZeroMemory(&rc, sizeof(rc));
		LVITEMINDEX lvii = { 0 };
		lvii.iItem = ListView_GetTopIndex(_list->hWnd()); // 1st visible item
		ListView_GetItemIndexRect(_list->hWnd(), &lvii, 0, LVIR_BOUNDS, &rc);
		int cyItem = rc.bottom - rc.top; // height of a single item
		int xTop = rc.top; // topmost X of 1st visible item

		SecureZeroMemory(&rc, sizeof(rc));
		SecureZeroMemory(&lvii, sizeof(lvii));
		lvii.iItem = this->i;
		ListView_GetItemIndexRect(_list->hWnd(), &lvii, 0, LVIR_BOUNDS, &rc);
		int xUs = rc.top; // our current X

		if(xUs < xTop || xUs > xTop + cyList) // if we're not visible
			ListView_Scroll(_list->hWnd(), 0, xUs - xTop - cyList / 2 + cyItem * 2);
	} else {
		ListView_EnsureVisible(_list->hWnd(), this->i, FALSE);
	}
	return *this;
}

String* ListView::Item::getText(String *pBuf, int iCol)
{
	// http://forums.codeguru.com/showthread.php?351972-Getting-listView-item-text-length
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.iSubItem = iCol;

	// Notice that, since Strings' size always increase, if the buffer
	// was previously allocated with a value bigger than 128, this will
	// speed up the size checks.
	
	int baseBufLen = 0;
	int retCode = 0;
	do {
		baseBufLen += 128; // buffer increasing step, arbitrary!
		pBuf->reserve(baseBufLen);
		lvi.cchTextMax = pBuf->reserved() + 1;
		lvi.pszText = pBuf->ptrAt(0);
		retCode = (int)_list->sendMessage(LVM_GETITEMTEXT, this->i, (LPARAM)&lvi);
	}
	while(retCode == pBuf->reserved()); // if could not get all chars, try again

	return pBuf;
}

ListView::Item& ListView::Item::setTextFmt(int iCol, const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String buf; buf.fmtv(fmt, args);
	va_end(args);
	this->setText(buf.str(), iCol);
	return *this;
}

LPARAM ListView::Item::getParam()
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_PARAM;

	ListView_GetItem(_list->hWnd(), &lvi);
	return lvi.lParam;
}

ListView::Item& ListView::Item::setParam(LPARAM lp)
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_PARAM;
	lvi.lParam = lp;

	ListView_SetItem(_list->hWnd(), &lvi);
	return *this;
}

int ListView::Item::getIcon()
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_IMAGE;

	ListView_GetItem(_list->hWnd(), &lvi);
	return lvi.iImage; // return index of icon within imagelist
}

ListView::Item& ListView::Item::setIcon(int iconIdx)
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iconIdx; // index of icon within imagelist

	ListView_SetItem(_list->hWnd(), &lvi);
	return *this;
}

ListView::Item ListView::Items::add(const wchar_t *caption, int iconIdx, int at)
{
	LVITEM lvi = { 0 };
	lvi.iItem = (at == -1 ? 0x0FFFFFFF : at);
	lvi.mask = LVIF_TEXT | (iconIdx == -1 ? 0 : LVIF_IMAGE);
	lvi.pszText = (wchar_t*)caption;
	lvi.iImage = iconIdx; // index of icon within imagelist

	return Item(ListView_InsertItem(_list->hWnd(), &lvi), _list); // return index of newly inserted item
}

ListView::Item ListView::Items::find(const wchar_t *caption)
{
	LVFINDINFO lfi = { 0 };
	lfi.flags = LVFI_STRING; // search is case-insensitive
	lfi.psz = caption;

	return Item(ListView_FindItem(_list->hWnd(), -1, &lfi), _list); // returns -1 if not found
}

void ListView::Items::select(const Array<int> *idx)
{
	// Select the items whose indexes have been passed in the array.
	for(int i = 0; i < idx->size(); ++i)
		ListView_SetItemState(_list->hWnd(), (*idx)[i], LVIS_SELECTED, LVIS_SELECTED);
}

void ListView::Items::removeSelected()
{
	_list->setRedraw(false);
	int i = -1;
	while((i = ListView_GetNextItem(_list->hWnd(), -1, LVNI_SELECTED)) != -1)
		ListView_DeleteItem(_list->hWnd(), i);
	_list->setRedraw(true);
}

void ListView::Items::getSelected(Array<int> *indexesBuf)
{
	indexesBuf->realloc(this->countSelected());
	int iBase = -1, iOutBuf = 0;

	for(;;) {
		iBase = ListView_GetNextItem(_list->hWnd(), iBase, LVNI_SELECTED);
		if(iBase == -1) break;
		(*indexesBuf)[iOutBuf++] = iBase;
	}
}

void ListView::Items::getSelectedText(Array<String> *captionsBuf, int iCol)
{
	captionsBuf->realloc(this->countSelected());
	int iBase = -1, iOutBuf = 0;

	for(;;) {
		iBase = ListView_GetNextItem(_list->hWnd(), iBase, LVNI_SELECTED);
		if(iBase == -1) break;
		(*this)[iBase].getText(&(*captionsBuf)[iOutBuf++], iCol); // subject to Item::getText() allocation routine
	}
}

void ListView::Items::getAllText(Array<String> *captionsBuf, int iCol)
{
	captionsBuf->realloc(this->count());
	for(int i = 0; i < captionsBuf->size(); ++i)
		(*this)[i].getText(&(*captionsBuf)[i], iCol); // subject to Item::getText() allocation routine
}

ListView& ListView::operator=(HWND hwnd)
{
	const int IDSUBCLASS = 1;

	if(this->hWnd()) // if previously assigned, remove previous subclassing
		RemoveWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS);

	*((Window*)this) = hwnd;
	SetWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS, (DWORD_PTR)this);
	items = Items(this); // initialize internal object
	_ctxMenuId = 0; // ID of context popup menu
	return *this;
}

ListView& ListView::iconPush(int iconId)
{
	HIMAGELIST hImg = this->_proceedImageList();
	HICON icon = (HICON)LoadImage(this->getInstance(),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	ImageList_AddIcon(hImg, icon);
	DestroyIcon(icon);
	return *this;
}

ListView& ListView::iconPush(const wchar_t *fileExtension)
{
	HIMAGELIST hImg = this->_proceedImageList();
	if(!hImg) {
		debug(L"ERROR: Imagelist creation failure.\n");
	} else {
		Icon expicon; // icon will be released at the end of this scope block
		expicon.getFromExplorer(fileExtension);
		ImageList_AddIcon(hImg, expicon.hIcon()); // append a clone of icon handle to imagelist
	}
	return *this; // return the index of the new icon
}

ListView& ListView::columnAdd(const wchar_t *caption, int cx)
{
	LVCOLUMN lvc = { 0 };

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = (wchar_t*)caption;
	lvc.cx = cx;

	ListView_InsertColumn(this->hWnd(), 0xFFFF, &lvc);
	return *this;
}

ListView& ListView::columnFit(int iCol)
{
	int numCols = this->columnCount();
	int cxUsed = 0;

	for(int i = 0; i < numCols; ++i) {
		if(i != iCol) {
			LVCOLUMN lvc = { 0 };
			lvc.mask = LVCF_WIDTH;
			ListView_GetColumn(this->hWnd(), i, &lvc); // retrieve cx of each column, except stretchee
			cxUsed += lvc.cx; // sum up
		}
	}

	RECT rc;
	this->getClientRect(&rc); // listview client area
	ListView_SetColumnWidth(this->hWnd(), iCol,
		rc.right /*- GetSystemMetrics(SM_CXVSCROLL)*/ - cxUsed); // fit the rest of available space
	return *this;
}

HIMAGELIST ListView::_proceedImageList()
{
	// Imagelist is destroyed automatically:
	// http://www.catch22.net/tuts/sysimgq
	// http://www.autohotkey.com/docs/commands/ListView.htm

	HIMAGELIST hImg = ListView_GetImageList(this->hWnd(), LVSIL_SMALL); // current imagelist
	if(!hImg) {
		hImg = ImageList_Create(16, 16, ILC_COLOR32, 1, 1); // create a 16x16 imagelist
		if(!hImg) return 0; // imagelist creation failure!
			ListView_SetImageList(this->hWnd(), hImg, LVSIL_SMALL); // associate imagelist to listview control
	}
	return hImg; // return handle to current imagelist
}

int ListView::_showCtxMenu(bool followCursor)
{
	if(!_ctxMenuId) return -1; // no context menu assigned via setContextMenu()

	POINT coords = { 0 };
	int itemBelowCursor = -1;

	if(followCursor) { // usually fired with a right-click
		LVHITTESTINFO lvhti = { 0 };
		GetCursorPos(&lvhti.pt); // relative to screen
		this->screenToClient(&lvhti.pt); // now relative to listview
		ListView_HitTest(this->hWnd(), &lvhti); // item below cursor, if any
		coords = lvhti.pt;
		itemBelowCursor = lvhti.iItem; // -1 if none
		if(itemBelowCursor != -1) { // an item was right-clicked
			if(!hasCtrl() && !hasShift()) {
				if((ListView_GetItemState(this->hWnd(), itemBelowCursor, LVIS_SELECTED) & LVIS_SELECTED) == 0) {
					// If right-clicked item isn't currently selected, unselect all and select just it.
					ListView_SetItemState(this->hWnd(), -1, 0, LVIS_SELECTED);
					ListView_SetItemState(this->hWnd(), itemBelowCursor, LVIS_SELECTED, LVIS_SELECTED);
				}
				ListView_SetItemState(this->hWnd(), itemBelowCursor, LVIS_FOCUSED, LVIS_FOCUSED); // focus clicked
			}
		} else { // no item was right-clicked
			if(!hasCtrl() && !hasShift())
				ListView_SetItemState(this->hWnd(), -1, 0, LVIS_SELECTED); // unselect all
		}
		this->setFocus(); // because a right-click won't set the focus by default
	} else { // usually fired with the context menu keyboard key
		int itemFocused = ListView_GetNextItem(this->hWnd(), -1, LVNI_FOCUSED);
		if(itemFocused != -1 && ListView_IsItemVisible(this->hWnd(), itemFocused)) { // item focused and visible
			RECT rcItem = { 0 };
			ListView_GetItemRect(this->hWnd(), itemFocused, &rcItem, LVIR_BOUNDS); // relative to listview
			coords.x = rcItem.left + 16;
			coords.y = rcItem.top + (rcItem.bottom - rcItem.top) / 2;
		} else { // no focused and visible item
			coords.x = 6;
			coords.y = 10;
		}
	}

	// The popup menu is created with hDlg as parent, so the menu messages go to it.
	// The lvhti coordinates are relative to listview, and will be mapped into screen-relative.
	popMenu(this->getParent().hWnd(), _ctxMenuId, coords.x, coords.y, this->hWnd());
	return itemBelowCursor; // -1 if none
}

LRESULT CALLBACK ListView::_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch(msg)
	{
	case WM_GETDLGCODE:
		if(lp && wp == 'A' && hasCtrl()) { // Ctrl+A to select all items
			((MSG*)lp)->wParam = 0; // prevent propagation, therefore beep
			ListView_SetItemState(hWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
			return DLGC_WANTCHARS;
		} else if(lp && wp == VK_RETURN) { // send Enter key to parent
			NMLVKEYDOWN nmlvkd = { { hWnd, GetDlgCtrlID(hWnd), LVN_KEYDOWN }, VK_RETURN, 0 };
			SendMessage(GetAncestor(hWnd, GA_PARENT), WM_NOTIFY, (WPARAM)hWnd, (LPARAM)&nmlvkd);
			((MSG*)lp)->wParam = 0; // prevent propagation, therefore beep
			return DLGC_WANTALLKEYS;
		} else if(lp && wp == VK_APPS) { // context menu keyboard key
			((ListView*)refData)->_showCtxMenu(false);
		}
		break;
	case WM_RBUTTONDOWN:
		((ListView*)refData)->_showCtxMenu(true);
		return 0;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _Proc, idSubclass);
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}