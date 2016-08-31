#include "../../../inc/slib/ui/check_box.h"

SLIB_UI_NAMESPACE_BEGIN

class _CheckBox_Icon : public Drawable
{
public:
	SafeRef<Pen> m_penBorder;
	SafeRef<Brush> m_brush;
	SafeRef<Pen> m_penCheck;
	
public:
	_CheckBox_Icon(const Ref<Pen>& penBorder, const Color& backColor, const Ref<Pen>& penCheck)
	{
		m_penBorder = penBorder;
		if (backColor.a > 0) {
			m_brush = Brush::createSolidBrush(backColor);
		}
		m_penCheck = penCheck;
	}
	
public:
	// override
	void onDrawAll(Canvas* canvas, const Rectangle& rectDst)
	{
		canvas->setAntiAlias(sl_false);
		canvas->drawRectangle(rectDst, m_penBorder, m_brush);
		canvas->setAntiAlias(sl_true);
		
		if (m_penCheck.isNotNull()) {
			Point pts[3];
			pts[0] = Point(0.2f, 0.6f);
			pts[1] = Point(0.4f, 0.8f);
			pts[2] = Point(0.8f, 0.3f);
			for (int i = 0; i < 3; i++) {
				pts[i].x = rectDst.left + rectDst.getWidth() * pts[i].x;
				pts[i].y = rectDst.top + rectDst.getHeight() * pts[i].y;
			}
			canvas->drawLines(pts, 3, m_penCheck);
		}
	}
	
};

class _CheckBox_Categories
{
public:
	ButtonCategory categories[2];
	
public:
	_CheckBox_Categories()
	{
		Color colorBackNormal = Color::White;
		Color colorBackHover = Color::White;
		Color colorBackDown(220, 230, 255);
		Color colorBackDisabled(220, 220, 220);
		Ref<Pen> penNormal = Pen::createSolidPen(1, Color::Black);
		Ref<Pen> penHover = Pen::createSolidPen(1, Color(0, 80, 200));
		Ref<Pen> penDown = penHover;
		Ref<Pen> penDisabled = Pen::createSolidPen(1, Color(90, 90, 90));
		Ref<Pen> penCheckNormal = Pen::createSolidPen(2, Color::Black);
		Ref<Pen> penCheckHover = Pen::createSolidPen(2, Color(0, 80, 200));
		Ref<Pen> penCheckDown = penCheckHover;
		Ref<Pen> penCheckDisabled = Pen::createSolidPen(2, Color(90, 90, 90));
		categories[0].properties[(int)ButtonState::Normal].textColor = Color::Black;
		categories[0].properties[(int)ButtonState::Normal].icon = new _CheckBox_Icon(penNormal, colorBackNormal, Ref<Pen>::null());
		categories[0].properties[(int)ButtonState::Disabled].textColor = Color(90, 90, 90);
		categories[0].properties[(int)ButtonState::Disabled].icon = new _CheckBox_Icon(penDisabled, colorBackDisabled, Ref<Pen>::null());
		categories[0].properties[(int)ButtonState::Hover].icon = new _CheckBox_Icon(penHover, colorBackHover, Ref<Pen>::null());
		categories[0].properties[(int)ButtonState::Down].icon = new _CheckBox_Icon(penDown, colorBackDown, Ref<Pen>::null());
		
		categories[1] = categories[0];
		categories[1].properties[(int)ButtonState::Normal].icon = new _CheckBox_Icon(penNormal, colorBackNormal, penCheckNormal);
		categories[1].properties[(int)ButtonState::Disabled].icon = new _CheckBox_Icon(penDisabled, colorBackDisabled, penCheckDisabled);
		categories[1].properties[(int)ButtonState::Hover].icon = new _CheckBox_Icon(penHover, colorBackHover, penCheckHover);
		categories[1].properties[(int)ButtonState::Down].icon = new _CheckBox_Icon(penDown, colorBackDown, penCheckDown);
	}
	
public:
	static ButtonCategory* getCategories()
	{
		SLIB_SAFE_STATIC(_CheckBox_Categories, ret)
		if (SLIB_SAFE_STATIC_CHECK_FREED(ret)) {
			return sl_null;
		}
		return ret.categories;
	}
};

SLIB_DEFINE_OBJECT(CheckBox, Button)

CheckBox::CheckBox() : CheckBox(2, _CheckBox_Categories::getCategories())
{
	setCreatingNativeWidget(sl_true);
}

CheckBox::CheckBox(sl_uint32 nCategories, ButtonCategory* categories) : Button(nCategories, categories)
{
	m_flagChecked = sl_false;
	setIconSize(Font::getDefaultFontSize());
	setIconMargin(2);
	setTextMargin(2, 0, 2, 2);
	setContentAlignment(Alignment::MiddleLeft);
	setIconAlignment(Alignment::MiddleLeft);
	setTextAlignment(Alignment::MiddleLeft);
}

sl_bool CheckBox::isChecked()
{
	if (isNativeWidget()) {
		_getChecked_NW();
	}
	return m_flagChecked;
}

void CheckBox::setChecked(sl_bool flag, sl_bool flagRedraw)
{
	m_flagChecked = flag;
	if (isNativeWidget()) {
		setCurrentCategory(flag ? 1 : 0, sl_false);
		_setChecked_NW(flag);
	} else {
		setCurrentCategory(flag ? 1 : 0, flagRedraw);
	}
}

void CheckBox::dispatchClick(UIEvent* ev)
{
	if (isNativeWidget()) {
		_getChecked_NW();
	} else {
		setChecked(!m_flagChecked);
	}
	Button::dispatchClick(ev);
}


#if !(defined(SLIB_PLATFORM_IS_OSX)) && !(defined(SLIB_PLATFORM_IS_WIN32))

Ref<ViewInstance> CheckBox::createNativeWidget(ViewInstance* parent)
{
	return Ref<ViewInstance>::null();
}

void CheckBox::_getChecked_NW()
{
}

void CheckBox::_setChecked_NW(sl_bool flag)
{
}

#endif


SLIB_UI_NAMESPACE_END

