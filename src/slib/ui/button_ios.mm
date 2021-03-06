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

#if defined(SLIB_UI_IS_IOS)

#include "slib/ui/button.h"

#include "view_ios.h"

@interface _priv_Slib_iOS_Button : UIButton {
	
	@public slib::WeakRef<slib::iOS_ViewInstance> m_viewInstance;

}

@end

namespace slib
{
	Ref<ViewInstance> Button::createNativeWidget(ViewInstance* _parent)
	{
		IOS_VIEW_CREATE_INSTANCE_BEGIN
		_priv_Slib_iOS_Button* handle = [[_priv_Slib_iOS_Button alloc] initWithFrame:frame];
		if (handle != nil) {
			[handle setTitle:(Apple::getNSStringFromString(m_text)) forState:UIControlStateNormal];
			Ref<Font> font = getFont();
			UIFont* hFont = GraphicsPlatform::getUIFont(font.get(), UIPlatform::getGlobalScaleFactor());
			if (hFont != nil) {
				handle.titleLabel.font = hFont;
			}
			[handle setTitleColor:[UIColor colorWithRed:0 green:0.4 blue:0.8 alpha:1] forState:UIControlStateNormal];
			[handle setTitleColor:[UIColor colorWithRed:0 green:0.2 blue:0.4 alpha:1] forState:UIControlStateHighlighted];
			[handle setTitleShadowColor:[UIColor colorWithRed:0.2 green:0.2 blue:0.2 alpha:1] forState:UIControlStateNormal];
		}
		IOS_VIEW_CREATE_INSTANCE_END
		return ret;
	}
	
	void Button::_setText_NW(const String& text)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), Button, _setText_NW, this, text));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UIButton class]]) {
			UIButton* v = (UIButton*)handle;
			[v setTitle: (Apple::getNSStringFromString(text)) forState:UIControlStateNormal];
		}
	}
	
	void Button::_setDefaultButton_NW(sl_bool flag)
	{
	}
	
	void Button::_setFont_NW(const Ref<Font>& font)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), Button, _setFont_NW, this, font));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UIButton class]]) {
			UIButton* v = (UIButton*)handle;
			UIFont* hFont = GraphicsPlatform::getUIFont(font.get(), UIPlatform::getGlobalScaleFactor());
			if (hFont != nil) {
				v.titleLabel.font = hFont;
			}
		}
	}
}

@implementation _priv_Slib_iOS_Button
-(id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self != nil) {
		[self addTarget:self action:@selector(onClick) forControlEvents:UIControlEventTouchUpInside];
	}
	return self;
}

-(void)onClick
{
	slib::Ref<slib::iOS_ViewInstance> instance = m_viewInstance;
	if (instance.isNotNull()) {
		instance->onClick();
	}
}
@end

#endif
