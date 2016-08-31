#include "../../../inc/slib/core/definition.h"

#if defined(SLIB_PLATFORM_IS_WIN32)

#include "../../../inc/slib/ui/button.h"

#include "view_win32.h"

SLIB_UI_NAMESPACE_BEGIN
class _Win32_ButtonViewInstance : public Win32_ViewInstance
{
public:
    // override
	sl_bool processWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
	{
		return sl_false;
	}

    // override
	sl_bool processCommand(SHORT code, LRESULT& result)
	{
		if (code == BN_CLICKED) {
			onClick();
			return sl_true;
		}
		return sl_false;
	}
};

Ref<ViewInstance> Button::createNativeWidget(ViewInstance* parent)
{
	Win32_UI_Shared* shared = Win32_UI_Shared::get();
	if (!shared) {
		return Ref<ViewInstance>::null();
	}
	String16 text = m_text;
	DWORD style = WS_TABSTOP;
	if (m_flagDefaultButton) {
		style |= BS_DEFPUSHBUTTON;
	}
	Ref<_Win32_ButtonViewInstance> ret = Win32_ViewInstance::create<_Win32_ButtonViewInstance>(this, parent, L"BUTTON", (LPCWSTR)(text.getData()), style, 0);
	if (ret.isNotNull()) {
		Ref<Font> font = getFont();
		Ref<FontInstance> fontInstance;
		HFONT hFont = UIPlatform::getGdiFont(font.ptr, fontInstance);
		if (hFont) {
			::SendMessageW(ret->getHandle(), WM_SETFONT, (WPARAM)hFont, TRUE);
		}
		_setFontInstance(fontInstance);
	}
	return ret;
}

void Button::_setText_NW(const String& text)
{
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		Windows::setWindowText(handle, text);
	}
}

void Button::_setDefaultButton_NW(sl_bool flag)
{
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		LONG old = ::GetWindowLongW(handle, GWL_STYLE);
		if (flag) {
			::SetWindowLongW(handle, GWL_STYLE, old | BS_DEFPUSHBUTTON);
		} else {
			::SetWindowLongW(handle, GWL_STYLE, old & (~(BS_DEFPUSHBUTTON)));
		}
		::SetWindowPos(handle, NULL, 0, 0, 0, 0
			, SWP_FRAMECHANGED | SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
	}
}

void Button::_setFont_NW(const Ref<Font>& font)
{
	Ref<FontInstance> fontInstance;
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		HFONT hFont = UIPlatform::getGdiFont(font.ptr, fontInstance);
		if (hFont) {
			::SendMessageW(handle, WM_SETFONT, (WPARAM)hFont, TRUE);
		}
	}
	_setFontInstance(fontInstance);
}

SLIB_UI_NAMESPACE_END

#endif
