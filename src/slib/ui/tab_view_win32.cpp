/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "slib/core/definition.h"

#if defined(SLIB_UI_IS_WIN32)

#include "slib/ui/tab_view.h"
#include "slib/ui/core.h"

#include "view_win32.h"

#include <commctrl.h>

namespace slib
{

	class _priv_TabView : public TabView
	{
	public:
		void _applyTabsCount(HWND hWnd)
		{
			ObjectLocker lock(this);
			sl_uint32 nOrig = (sl_uint32)(::SendMessageW(hWnd, TCM_GETITEMCOUNT, 0, 0));
			sl_uint32 nNew = (sl_uint32)(m_items.getCount());
			if (nOrig == nNew) {
				return;
			}
			if (nOrig > nNew) {
				if (nNew > 0) {
					for (sl_uint32 i = nOrig; i > nNew; i--) {
						::SendMessageW(hWnd, TCM_GETITEMCOUNT, (WPARAM)(i - 1), 0);
					}
				} else {
					::SendMessageW(hWnd, TCM_DELETEALLITEMS, 0, 0);
				}
			} else {
				for (sl_uint32 i = nOrig; i < nNew; i++) {
					TCITEMW tci;
					Base::zeroMemory(&tci, sizeof(tci));
					::SendMessageW(hWnd, TCM_INSERTITEM, (WPARAM)i, (LPARAM)(&tci));
				}
			}
		}

		void _copyTabs(HWND hWnd, ViewInstance* viewInstance)
		{
			_applyTabsCount(hWnd);
			ListLocker<TabViewItem> items(m_items);
			for (sl_size i = 0; i < items.count; i++) {
				TCITEMW tci;
				Base::zeroMemory(&tci, sizeof(tci));
				tci.mask = TCIF_TEXT;
				String16 label = items[i].label;
				tci.pszText = (LPWSTR)(label.getData());
				::SendMessageW(hWnd, TCM_SETITEMW, (WPARAM)i, (LPARAM)&tci);
			}
			_selectTab(hWnd, viewInstance, m_indexSelected);
		}

		void _setTabLabel(HWND hWnd, sl_uint32 index, const String& _label)
		{
			TCITEMW tci;
			Base::zeroMemory(&tci, sizeof(tci));
			tci.mask = TCIF_TEXT;
			String16 label = _label;
			tci.pszText = (LPWSTR)(label.getData());
			::SendMessageW(hWnd, TCM_SETITEMW, (WPARAM)index, (LPARAM)(&tci));
		}

		void _selectTab(HWND hWnd, ViewInstance* viewInstance, sl_uint32 index)
		{
			sl_uint32 n = (sl_uint32)(m_items.getCount());
			if (index >= n) {
				index = 0;
			}
			::SendMessageW(hWnd, TCM_SETCURSEL, (WPARAM)m_indexSelected, 0);
			_applyTabContents(hWnd, viewInstance);
		}

		sl_uint32 _getSelectedIndex(HWND hWnd)
		{
			return (sl_uint32)(::SendMessageW(hWnd, TCM_GETCURSEL, 0, 0));
		}

		void _onSelectTab(HWND hWnd, ViewInstance* viewInstance)
		{
			sl_uint32 index = _getSelectedIndex(hWnd);
			dispatchSelectTab(index);
			_applyTabContents(hWnd, viewInstance);
		}

		void _applyTabContents(HWND hWnd, ViewInstance* viewInstance)
		{
			UIRect rc = _getClientBounds(hWnd);
			sl_size sel = m_indexSelected;
			ListLocker<TabViewItem> items(m_items);
			for (sl_size i = 0; i < items.count; i++) {
				Ref<View> view = items[i].contentView;
				if (view.isNotNull()) {
					view->setFrame(rc);
					if (i == sel) {
						view->setVisible(sl_true);
						if (view->getViewInstance().isNull()) {
							view->attachToNewInstance(viewInstance);
						}
					} else {
						view->setVisible(sl_false);
					}
				}
			}
		}

		void _applyClientBounds(HWND hWnd)
		{
			UIRect rc = _getClientBounds(hWnd);
			ListLocker<TabViewItem> items(m_items);
			for (sl_size i = 0; i < items.count; i++) {
				Ref<View> view = items[i].contentView;
				if (view.isNotNull()) {
					view->setFrame(rc);
				}
			}
		}

		UIRect _getClientBounds(HWND hWnd)
		{
			RECT rc;
			rc.left = -2;
			rc.top = 0;
			rc.right = (int)(getWidth());
			rc.bottom = (int)(getHeight()) + 1;
			::SendMessageW(hWnd, TCM_ADJUSTRECT, FALSE, (LPARAM)(&rc));
			return UIRect((sl_ui_pos)(rc.left), (sl_ui_pos)(rc.top), (sl_ui_pos)(rc.right), (sl_ui_pos)(rc.bottom));
		}
	};

	class _priv_Win32_TabViewInstance : public Win32_ViewInstance
	{
	public:
		sl_bool processWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override
		{
			return sl_false;
		}

		sl_bool processCommand(SHORT code, LRESULT& result) override
		{
			return sl_false;
		}

		sl_bool processNotify(NMHDR* nmhdr, LRESULT& result) override
		{
			HWND handle = getHandle();
			if (handle) {
				Ref<View> _view = getView();
				if (_priv_TabView* view = CastInstance<_priv_TabView>(_view.get())) {
					UINT code = nmhdr->code;
					if (code == TCN_SELCHANGE) {
						view->_onSelectTab(handle, this);
						return sl_true;
					}
				}
			}
			return sl_false;
		}
	};

	Ref<ViewInstance> TabView::createNativeWidget(ViewInstance* parent)
	{
		Win32_UI_Shared* shared = Win32_UI_Shared::get();
		if (!shared) {
			return sl_null;
		}

		DWORD style = WS_CLIPCHILDREN;
		DWORD styleEx = WS_EX_CONTROLPARENT;

		Ref<_priv_Win32_TabViewInstance> ret = Win32_ViewInstance::create<_priv_Win32_TabViewInstance>(this, parent, L"SysTabControl32", L"", style, styleEx);
		
		if (ret.isNotNull()) {

			HWND handle = ret->getHandle();

			Ref<Font> font = getFont();
			HFONT hFont = GraphicsPlatform::getGdiFont(font.get());
			if (hFont) {
				::SendMessageW(handle, WM_SETFONT, (WPARAM)hFont, TRUE);
			}

			((_priv_TabView*)this)->_copyTabs(handle, ret.get());
		}
		return ret;
	}

	void TabView::_refreshTabsCount_NW()
	{
		HWND handle = UIPlatform::getViewHandle(this);
		if (handle) {
			((_priv_TabView*)this)->_applyTabsCount(handle);
		}
	}

	void TabView::_refreshSize_NW()
	{
		HWND handle = UIPlatform::getViewHandle(this);
		if (handle) {
			((_priv_TabView*)this)->_applyClientBounds(handle);
		}
	}

	void TabView::_setTabLabel_NW(sl_uint32 index, const String& text)
	{
		HWND handle = UIPlatform::getViewHandle(this);
		if (handle) {
			((_priv_TabView*)this)->_setTabLabel(handle, index, text);
		}
	}

	void TabView::_setTabContentView_NW(sl_uint32 index, const Ref<View>& view)
	{
		Ref<ViewInstance> viewInstance = getViewInstance();
		if (viewInstance.isNotNull()) {
			HWND handle = UIPlatform::getViewHandle(viewInstance.get());
			((_priv_TabView*)this)->_applyTabContents(handle, viewInstance.get());
		}
	}

	void TabView::_selectTab_NW(sl_uint32 index)
	{
		Ref<ViewInstance> viewInstance = getViewInstance();
		if (viewInstance.isNotNull()) {
			HWND handle = UIPlatform::getViewHandle(viewInstance.get());
			((_priv_TabView*)this)->_selectTab(handle, viewInstance.get(), index);
		}
	}

	UISize TabView::_getContentViewSize_NW()
	{
		HWND handle = UIPlatform::getViewHandle(this);
		if (handle) {
			return ((_priv_TabView*)this)->_getClientBounds(handle).getSize();
		}
		return UISize::zero();
	}

	void TabView::_setFont_NW(const Ref<Font>& font)
	{
		HWND handle = UIPlatform::getViewHandle(this);
		if (handle) {
			HFONT hFont = GraphicsPlatform::getGdiFont(font.get());
			if (hFont) {
				::SendMessageW(handle, WM_SETFONT, (WPARAM)hFont, TRUE);
			}
		}
	}

}

#endif
