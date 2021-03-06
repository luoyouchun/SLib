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

#include "slib/ui/label_view.h"

#include "view_ios.h"

@interface _priv_Slib_iOS_LabelTextField : UILabel {
	
	@public slib::WeakRef<slib::iOS_ViewInstance> m_viewInstance;
	
}
@end

namespace slib
{
	class _priv_LabelView : public LabelView
	{
	public:
		void applyProperties(UILabel* handle)
		{
			[handle setText:(Apple::getNSStringFromString(m_text))];
			[handle setTextAlignment:translateAlignment(m_textAlignment)];
			if (isBorder()) {
				[handle.layer setBorderColor:([[UIColor grayColor] CGColor])];
				[handle.layer setBorderWidth:1];
			} else {
				[handle.layer setBorderWidth:0];
			}
			[handle setTextColor:(GraphicsPlatform::getUIColorFromColor(m_textColor))];
			[handle setBackgroundColor:(GraphicsPlatform::getUIColorFromColor(getBackgroundColor()))];
			Ref<Font> font = getFont();
			UIFont* hFont = GraphicsPlatform::getUIFont(font.get(), UIPlatform::getGlobalScaleFactor());
			if (hFont != nil) {
				[handle setFont:hFont];
			}
		}
		
		static NSTextAlignment translateAlignment(Alignment _align)
		{
			Alignment align = _align & Alignment::HorizontalMask;
			if (align == Alignment::Center) {
				return NSTextAlignmentCenter;
			} else if (align == Alignment::Right) {
				return NSTextAlignmentRight;
			}
			return NSTextAlignmentLeft;
		}
		
	};
	
	Ref<ViewInstance> LabelView::createNativeWidget(ViewInstance* _parent)
	{
		IOS_VIEW_CREATE_INSTANCE_BEGIN
		_priv_Slib_iOS_LabelTextField* handle = [[_priv_Slib_iOS_LabelTextField alloc] initWithFrame:frame];
		if (handle != nil) {
			((_priv_LabelView*)this)->applyProperties(handle);
		}
		IOS_VIEW_CREATE_INSTANCE_END
		return ret;
	}
	
	void LabelView::_setText_NW(const String& value)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), LabelView, _setText_NW, this, value));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UILabel class]]) {
			UILabel* tv = (UILabel*)handle;
			[tv setText:(Apple::getNSStringFromString(value))];
		}
	}
	
	void LabelView::_setTextAlignment_NW(Alignment align)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), LabelView, _setTextAlignment_NW, this, align));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UILabel class]]) {
			UILabel* tv = (UILabel*)handle;
			[tv setTextAlignment:_priv_LabelView::translateAlignment(align)];
		}
	}
	
	void LabelView::_setTextColor_NW(const Color& color)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), LabelView, _setTextColor_NW, this, color));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UILabel class]]) {
			UILabel* tv = (UILabel*)handle;
			[tv setTextColor:(GraphicsPlatform::getUIColorFromColor(color))];
		}
	}
	
	void LabelView::_setFont_NW(const Ref<Font>& font)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), LabelView, _setFont_NW, this, font));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UILabel class]]) {
			UILabel* tv = (UILabel*)handle;
			UIFont* hFont = GraphicsPlatform::getUIFont(font.get(), UIPlatform::getGlobalScaleFactor());
			if (hFont != nil) {
				[tv setFont:hFont];
			}
		}
	}
	
	void LabelView::_setBorder_NW(sl_bool flag)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), LabelView, _setBorder_NW, this, flag));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UILabel class]]) {
			UILabel* tv = (UILabel*)handle;
			if (flag) {
				[tv.layer setBorderColor:([[UIColor grayColor] CGColor])];
				[tv.layer setBorderWidth:1];
			} else {
				[tv.layer setBorderWidth:0];
			}
		}
	}
	
	void LabelView::_setBackgroundColor_NW(const Color& color)
	{
		if (!(isUiThread())) {
			dispatchToUiThread(SLIB_BIND_WEAKREF(void(), LabelView, _setBackgroundColor_NW, this, color));
			return;
		}
		UIView* handle = UIPlatform::getViewHandle(this);
		if (handle != nil && [handle isKindOfClass:[UILabel class]]) {
			UILabel* tv = (UILabel*)handle;
			[tv setBackgroundColor:(GraphicsPlatform::getUIColorFromColor(color))];
		}
	}	
}

@implementation _priv_Slib_iOS_LabelTextField
-(id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self != nil) {
		UITapGestureRecognizer *tapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(labelTapped)];
		tapGestureRecognizer.numberOfTapsRequired = 1;
		[self addGestureRecognizer:tapGestureRecognizer];
		self.userInteractionEnabled = YES;
	}
	return self;
}

- (void)labelTapped
{
	slib::Ref<slib::iOS_ViewInstance> instance = m_viewInstance;
	if (instance.isNotNull()) {
		instance->onClick();
	}
}
@end

#endif
