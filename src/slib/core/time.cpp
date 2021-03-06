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

#include "slib/core/time.h"

#include "slib/core/variant.h"
#include "slib/core/string_buffer.h"
#include "slib/core/safe_static.h"

#define TIME_MILLIS SLIB_INT64(1000)
#define TIME_MILLISF 1000.0
#define TIME_SECOND SLIB_INT64(1000000)
#define TIME_SECONDF 1000000.0
#define TIME_MINUTE SLIB_INT64(60000000)
#define TIME_MINUTEF 60000000.0
#define TIME_HOUR SLIB_INT64(3600000000)
#define TIME_HOURF 3600000000.0
#define TIME_DAY SLIB_INT64(86400000000)
#define TIME_DAYF 86400000000.0

#if defined(SLIB_PLATFORM_IS_WINDOWS)
#include <windows.h>
#elif defined(SLIB_PLATFORM_IS_UNIX)
#include <sys/time.h>
#include <time.h>
#endif

namespace slib
{
	
	SLIB_DEFINE_OBJECT(CTimeZone, Object)
	
	CTimeZone::CTimeZone() noexcept
	{
	}
	
	CTimeZone::~CTimeZone() noexcept
	{
	}

	sl_int64 CTimeZone::getOffset()
	{
		return getOffset(Time::now());
	}
	
	
	SLIB_DEFINE_OBJECT(GenericTimeZone, CTimeZone)
	
	GenericTimeZone::GenericTimeZone(sl_int64 offset)
	{
		m_offset = offset;
	}
	
	GenericTimeZone::~GenericTimeZone()
	{
	}
	
	sl_int64 GenericTimeZone::getOffset(const Time& time)
	{
		return m_offset;
	}
	

	SLIB_STATIC_ZERO_INITIALIZED(TimeZone, _g_timezone_Local)
	const TimeZone& TimeZone::Local = _g_timezone_Local;
	
	const TimeZone& TimeZone::UTC() noexcept
	{
		SLIB_SAFE_STATIC(TimeZone, utc, TimeZone::create(0));
		return utc;
	}
	
	TimeZone TimeZone::create(sl_int64 offset) noexcept
	{
		return new GenericTimeZone(offset);
	}

	sl_bool TimeZone::isLocal() const noexcept
	{
		return isNull();
	}
	
	sl_bool TimeZone::isUTC() const noexcept
	{
		return ref._ptr == UTC().ref._ptr;
	}
	
	sl_int64 TimeZone::getOffset() const
	{
		return getOffset(Time::now());
	}
	
	sl_int64 TimeZone::getOffset(const Time &time) const
	{
		Ref<CTimeZone> obj = ref;
		if (obj.isNotNull()) {
			if (isUTC()) {
				return 0;
			}
			return obj->getOffset();
		}
		return time.getLocalTimeOffset();
	}
	
	
	sl_bool Atomic<TimeZone>::isLocal() const noexcept
	{
		return isNull();
	}
	
	sl_bool Atomic<TimeZone>::isUTC() const noexcept
	{
		return ref._ptr == TimeZone::UTC().ref._ptr;
	}
	
	sl_int64 Atomic<TimeZone>::getOffset() const
	{
		return getOffset(Time::now());
	}
	
	sl_int64 Atomic<TimeZone>::getOffset(const Time &time) const
	{
		return TimeZone(*this).getOffset(time);
	}
	
	
	TimeComponents::TimeComponents() noexcept
	{
		Base::zeroMemory(this, sizeof(TimeComponents));
	}
	

	Time::Time(int year, int month, int date, const TimeZone& zone) noexcept
	{
		set(year, month, date, 0, 0, 0, 0, 0, zone);
	}

	Time::Time(int year, int month, int date, int hour, int minute, int second, int milliseconds, int microseconds, const TimeZone& zone) noexcept
	{
		set(year, month, date, hour, minute, second, milliseconds, microseconds, zone);
	}

	Time::Time(const TimeComponents& comps, const TimeZone& zone) noexcept
	{
		set(comps, zone);
	}

	Time::Time(const String& str, const TimeZone& zone) noexcept
	{
		setString(str, zone);
	}

	Time::Time(const String16& str, const TimeZone& zone) noexcept
	{
		setString(str, zone);
	}

	Time::Time(const AtomicString& str, const TimeZone& zone) noexcept
	{
		setString(str, zone);
	}

	Time::Time(const AtomicString16& str, const TimeZone& zone) noexcept
	{
		setString(str, zone);
	}

	Time::Time(const sl_char8* str, const TimeZone& zone) noexcept
	{
		setString(str, zone);
	}

	Time::Time(const sl_char16* str, const TimeZone& zone) noexcept
	{
		setString(str, zone);
	}

	Time Time::now() noexcept
	{
		Time ret;
		ret.setNow();
		return ret;
	}

	void Time::setInt(sl_int64 time) noexcept
	{
		m_time = time;
	}

	void Time::add(sl_int64 time) noexcept
	{
		m_time += time;
	}

	void Time::add(const Time& other) noexcept
	{
		m_time += other.m_time;
	}

	void Time::setNow() noexcept
	{
		_setNow();
	}
	
	sl_bool Time::setSystemTime(const Time& time) noexcept
	{
		return time._setToSystem();
	}

	void Time::get(TimeComponents& output, const TimeZone& zone) const noexcept
	{
		if (zone.isNull()) {
			if (!(_get(output, sl_false))) {
				Base::zeroMemory(&output, sizeof(output));
				return;
			}
		} else if (zone.isUTC()) {
			if (!(_get(output, sl_true))) {
				Base::zeroMemory(&output, sizeof(output));
				return;
			}
		} else {
			sl_int64 offset = zone.getOffset(*this);
			Time t(m_time + offset * TIME_SECOND);
			if (!(t._get(output, sl_true))) {
				Base::zeroMemory(&output, sizeof(output));
				return;
			}
		}
		sl_int64 t = m_time % TIME_SECOND;
		output.milliseconds = (sl_uint16)(t / TIME_MILLIS);
		output.microseconds = (sl_uint16)(t % TIME_MILLIS);
	}
	
	void Time::getUTC(TimeComponents& output) const noexcept
	{
		get(output, TimeZone::UTC());
	}
	
	void Time::set(const TimeComponents& comps, const TimeZone& zone) noexcept
	{
		set(comps.year, comps.month, comps.day, comps.hour, comps.minute, comps.second, comps.milliseconds, comps.microseconds, zone);
	}

	void Time::setUTC(const TimeComponents& comps) noexcept
	{
		set(comps, TimeZone::UTC());
	}
	
	void Time::set(int year, int month, int day, int hour, int minute, int second, int milliseconds, int microseconds, const TimeZone& zone) noexcept
	{
		if (year == 0 && month == 0 && day == 0) {
			m_time = hour * TIME_HOUR + minute * TIME_MINUTE + second * TIME_SECOND;
			return;
		}
		if (month <= 0) {
			month = 1;
		}
		if (day <= 0) {
			day = 1;
		}
		sl_int64 t;
		if (zone.isNull()) {
			t = _set(year, month, day, hour, minute, second, sl_false);
		} else if (zone.isUTC()) {
			t = _set(year, month, day, hour, minute, second, sl_true);
		} else {
			t = _set(year, month, day, hour, minute, second, sl_true);
			t -= zone.getOffset(t) * TIME_SECOND;
		}
		m_time = t + milliseconds * 1000 + microseconds;
	}

	void Time::setUTC(int year, int month, int day, int hour, int minute, int second, int milliseconds, int microseconds) noexcept
	{
		set(year, month, day, hour, minute, second, milliseconds, microseconds, TimeZone::UTC());
	}
	
	void Time::setDate(int year, int month, int day, const TimeZone& zone) noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		comps.year = year;
		comps.month = month;
		comps.day = day;
		set(comps, zone);
	}

	int Time::getYear(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.year;
	}

	void Time::setYear(int year, const TimeZone& zone) noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		comps.year = year;
		set(comps, zone);
	}

	void Time::addYears(int years, const TimeZone& zone) noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		comps.year += years;
		set(comps, zone);
	}

	int Time::getMonth(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.month;
	}

	void Time::setMonth(int month, const TimeZone& zone) noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		int monthNew = (month - 1) % 12;
		int yearAdd = (month - 1) / 12;
		if (monthNew < 0) {
			monthNew += 12;
			yearAdd--;
		}
		comps.year += yearAdd;
		comps.month = monthNew + 1;
		set(comps, zone);
	}

	void Time::addMonths(int months, const TimeZone& zone) noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		months += comps.month;
		int monthNew = (months - 1) % 12;
		int yearAdd = (months - 1) / 12;
		if (monthNew < 0) {
			monthNew += 12;
			yearAdd--;
		}
		comps.year += yearAdd;
		comps.month = monthNew + 1;
		set(comps, zone);
	}

	int Time::getDay(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.day;
	}

	void Time::setDay(int day, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)(day - getDay(zone))*TIME_DAY;
	}

	void Time::addDays(sl_int64 days) noexcept
	{
		m_time += days*TIME_DAY;
	}

	double Time::getDayf(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.day + (comps.hour * TIME_HOUR + comps.minute * TIME_MINUTE + comps.second * TIME_SECOND + comps.milliseconds * TIME_MILLIS + comps.microseconds) / TIME_DAYF;
	}

	void Time::setDayf(double day, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)((day - getDayf(zone))*TIME_DAYF);
	}

	void Time::addDaysf(double days) noexcept
	{
		m_time += (sl_int64)(days*TIME_DAYF);
	}

	int Time::getHour(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.hour;
	}

	void Time::setHour(int hour, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)(hour - getHour(zone))*TIME_HOUR;
	}

	void Time::addHours(sl_int64 hours) noexcept
	{
		m_time += hours*TIME_HOUR;
	}

	double Time::getHourf(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.hour + (comps.minute * TIME_MINUTE + comps.second * TIME_SECOND + comps.milliseconds * TIME_MILLIS + comps.microseconds) / TIME_HOURF;
	}

	void Time::setHourf(double hour, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)((hour - getHourf(zone))*TIME_HOURF);
	}

	void Time::addHoursf(double hours) noexcept
	{
		m_time += (sl_int64)(hours*TIME_HOURF);
	}

	int Time::getMinute(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.minute;
	}

	void Time::setMinute(int minute, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)(minute - getMinute(zone))*TIME_MINUTE;
	}

	void Time::addMinutes(sl_int64 minutes) noexcept
	{
		m_time += minutes*TIME_MINUTE;
	}

	double Time::getMinutef(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.minute + (comps.second * TIME_SECOND + comps.milliseconds * TIME_MILLIS + comps.microseconds) / TIME_MINUTEF;
	}

	void Time::setMinutef(double minute, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)((minute - getMinutef(zone))*TIME_MINUTEF);
	}

	void Time::addMinutesf(double minutes) noexcept
	{
		m_time += (sl_int64)(minutes*TIME_MINUTEF);
	}

	int Time::getSecond(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.second;
	}

	void Time::setSecond(int second, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)(second - getSecond(zone))*TIME_SECOND;
	}

	void Time::addSeconds(sl_int64 seconds) noexcept
	{
		m_time += seconds*TIME_SECOND;
	}

	double Time::getSecondf(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.second + (comps.milliseconds * TIME_MILLIS + comps.microseconds) / TIME_SECONDF;
	}

	void Time::setSecondf(double second, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)((second - getSecondf(zone))*TIME_SECONDF);
	}

	void Time::addSecondsf(double seconds) noexcept
	{
		m_time += (sl_int64)(seconds*TIME_SECONDF);
	}

	int Time::getMillisecond() const noexcept
	{
		int n = (int)(m_time % TIME_SECOND);
		if (n < 0) {
			n += TIME_SECOND;
		}
		return n / TIME_MILLIS;
	}

	void Time::setMillisecond(int millis) noexcept
	{
		m_time += (sl_int64)(millis - getMillisecond())*TIME_MILLIS;
	}

	void Time::addMilliseconds(sl_int64 milis) noexcept
	{
		m_time += milis*TIME_MILLIS;
	}

	double Time::getMillisecondf() const noexcept
	{
		int n = (int)(m_time % TIME_SECOND);
		if (n < 0) {
			n += TIME_SECOND;
		}
		return n / TIME_MILLISF;
	}

	void Time::setMillisecondf(double millis) noexcept
	{
		m_time += (sl_int64)((millis - getMillisecondf())*TIME_MILLIS);
	}

	void Time::addMillisecondsf(double milis) noexcept
	{
		m_time += (sl_int64)(milis*TIME_MILLISF);
	}

	int Time::getMicrosecond() const noexcept
	{
		int n = (int)(m_time % TIME_MILLIS);
		if (n < 0) {
			n += TIME_MILLIS;
		}
		return n;
	}

	void Time::setMicrosecond(int micros) noexcept
	{
		m_time += (micros - getMicrosecond());
	}

	void Time::addMicroseconds(sl_int64 micros) noexcept
	{
		m_time += micros;
	}

	double Time::getMicrosecondf() const noexcept
	{
		return (double)(getMicrosecond());
	}

	void Time::setMicrosecondf(double micros) noexcept
	{
		setMicrosecond((int)micros);
	}

	void Time::addMicrosecondsf(double micros) noexcept
	{
		m_time += (sl_int64)micros;
	}

	int Time::getDayOfWeek(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.dayOfWeek;
	}

	void Time::setDayOfWeek(int day, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)(day - getDayOfWeek(zone))*TIME_DAY;
	}

	int Time::getDayOfYear(const TimeZone& zone) const noexcept
	{
		return (int)((m_time - Time(getYear(zone), 1, 1, zone).m_time) / TIME_DAY) + 1;
	}

	void Time::setDayOfYear(int day, const TimeZone& zone) noexcept
	{
		m_time += (sl_int64)(day - getDayOfYear(zone))*TIME_DAY;
	}

	sl_int64 Time::getDaysCount() const noexcept
	{
		return (m_time / TIME_DAY);
	}

	void Time::setDaysCount(sl_int64 days) noexcept
	{
		m_time = days*TIME_DAY + (m_time % TIME_DAY);
	}

	double Time::getDaysCountf() const noexcept
	{
		return (m_time / TIME_DAYF);
	}

	void Time::setDaysCountf(double days) noexcept
	{
		m_time = (sl_int64)(days*TIME_DAYF);
	}

	sl_int64 Time::getHoursCount() const noexcept
	{
		return (m_time / TIME_HOUR);
	}

	void Time::setHoursCount(sl_int64 hours) noexcept
	{
		m_time = hours*TIME_HOUR + (m_time % TIME_HOUR);
	}

	double Time::getHoursCountf() const noexcept
	{
		return (m_time / TIME_HOURF);
	}

	void Time::setHoursCountf(double hours) noexcept
	{
		m_time = (sl_int64)(hours*TIME_HOURF);
	}

	sl_int64 Time::getMinutesCount() const noexcept
	{
		return (m_time / TIME_MINUTE);
	}

	void Time::setMinutesCount(sl_int64 minutes) noexcept
	{
		m_time = minutes*TIME_MINUTE + (m_time % TIME_MINUTE);
	}

	double Time::getMinutesCountf() const noexcept
	{
		return (m_time / TIME_MINUTEF);
	}

	void Time::setMinutesCountf(double minutes) noexcept
	{
		m_time = (sl_int64)(minutes*TIME_MINUTEF);
	}

	sl_int64 Time::getSecondsCount() const noexcept
	{
		return (m_time / TIME_SECOND);
	}

	void Time::setSecondsCount(sl_int64 seconds) noexcept
	{
		m_time = seconds*TIME_SECOND + (m_time % TIME_SECOND);
	}

	double Time::getSecondsCountf() const noexcept
	{
		return (m_time / TIME_SECONDF);
	}

	void Time::setSecondsCountf(double seconds) noexcept
	{
		m_time = (sl_int64)(seconds*TIME_SECONDF);
	}

	sl_int64 Time::getMillisecondsCount() const noexcept
	{
		return (m_time / TIME_MILLIS);
	}

	void Time::setMillisecondsCount(sl_int64 millis) noexcept
	{
		m_time = millis*TIME_MILLIS + (m_time % TIME_MILLIS);
	}

	double Time::getMillisecondsCountf() const noexcept
	{
		return (m_time / TIME_MILLISF);
	}

	void Time::setMillisecondsCountf(double millis) noexcept
	{
		m_time = (sl_int64)(millis*TIME_MILLISF);
	}

	sl_int64 Time::getMicrosecondsCount() const noexcept
	{
		return m_time;
	}

	void Time::setMicrosecondsCount(sl_int64 micros) noexcept
	{
		m_time = micros;
	}

	double Time::getMicrosecondsCountf() const noexcept
	{
		return (double)m_time;
	}

	void Time::setMicrosecondsCountf(double micros) noexcept
	{
		m_time = (sl_int64)(micros);
	}
	
	sl_int64 Time::getLocalTimeOffset() const noexcept
	{
		Time o = m_time / TIME_SECOND * TIME_SECOND;
		TimeComponents comps;
		o._get(comps, sl_false);
		Time t = Time::_set(comps.year, comps.month, comps.day, comps.hour, comps.minute, comps.second, sl_true);
		return (t - o).getSecondsCount();
	}

	int Time::getDaysCountInMonth(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		Time timeStart(comps.year, comps.month, 1, zone);
		Time timeEnd = timeStart;
		timeEnd.addMonths(1, zone);
		return (int)((timeEnd.m_time - timeStart.m_time) / TIME_DAY);
	}

	int Time::getDaysCountInYear(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		Time timeStart(comps.year, 1, 1, zone);
		Time timeEnd(comps.year + 1, 1, 1, zone);
		return (int)((timeEnd.m_time - timeStart.m_time) / TIME_DAY);
	}

	int Time::getQuarter(const TimeZone& zone) const noexcept
	{
		return ((getMonth(zone) - 1) / 3) + 1;
	}

	String Time::getWeekday(sl_bool flagShort, const TimeZone& zone) const noexcept
	{
		switch (getDayOfWeek(zone)) {
		case 0:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Sun");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Sunday");
				return s;
			}
		case 1:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Mon");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Monday");
				return s;
			}
		case 2:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Tue");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Tuesday");
				return s;
			}
		case 3:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Wed");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Wednesday");
				return s;
			}
		case 4:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Thu");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Thursday");
				return s;
			}
		case 5:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Fri");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Friday");
				return s;
			}
		case 6:
			if (flagShort) {
				SLIB_STATIC_STRING(s, "Sat");
				return s;
			} else {
				SLIB_STATIC_STRING(s, "Saturday");
				return s;
			}
		}
		return sl_null;
	}

	String Time::getWeekday(const TimeZone& zone) const noexcept
	{
		return getWeekday(sl_false, zone);
	}
	
	Time Time::getTimeOnly(const TimeZone& zone) const noexcept
	{
		TimeComponents comps;
		get(comps, zone);
		return comps.hour * TIME_HOUR + comps.minute * TIME_MINUTE + comps.second * TIME_SECOND + comps.milliseconds * TIME_MILLIS + comps.microseconds;
	}

	String Time::toString(const TimeZone& zone) const noexcept
	{
		TimeComponents d;
		get(d, zone);
		StringBuffer sb;
		sb.add(String::fromInt32(d.year, 10, 4));
		sb.addStatic("-", 1);
		sb.add(String::fromInt32(d.month, 10, 2));
		sb.addStatic("-", 1);
		sb.add(String::fromInt32(d.day, 10, 2));
		sb.addStatic(" ", 1);
		sb.add(String::fromInt32(d.hour, 10, 2));
		sb.addStatic(":", 1);
		sb.add(String::fromInt32(d.minute, 10, 2));
		sb.addStatic(":", 1);
		sb.add(String::fromInt32(d.second, 10, 2));
		return sb.merge();
	}

	String16 Time::toString16(const TimeZone& zone) const noexcept
	{
		TimeComponents d;
		get(d, zone);
		StringBuffer16 sb;
		sb.add(String16::fromInt32(d.year, 10, 4));
		{
			const sl_char16 ch = '-';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.month, 10, 2));
		{
			const sl_char16 ch = '-';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.day, 10, 2));
		{
			const sl_char16 ch = ' ';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.hour, 10, 2));
		{
			const sl_char16 ch = ':';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.minute, 10, 2));
		{
			const sl_char16 ch = ':';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.second, 10, 2));
		return sb.merge();
	}

	String Time::getDateString(const TimeZone& zone) const noexcept
	{
		TimeComponents d;
		get(d, zone);
		StringBuffer sb;
		sb.add(String::fromInt32(d.year, 10, 4));
		sb.addStatic("-", 1);
		sb.add(String::fromInt32(d.month, 10, 2));
		sb.addStatic("-", 1);
		sb.add(String::fromInt32(d.day, 10, 2));
		return sb.merge();
	}

	String16 Time::getDateString16(const TimeZone& zone) const noexcept
	{
		TimeComponents d;
		get(d, zone);
		StringBuffer16 sb;
		sb.add(String16::fromInt32(d.year, 10, 4));
		{
			const sl_char16 ch = '-';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.month, 10, 2));
		{
			const sl_char16 ch = '-';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.day, 10, 2));
		return sb.merge();
	}

	String Time::getTimeString(const TimeZone& zone) const noexcept
	{
		TimeComponents d;
		get(d, zone);
		StringBuffer sb;
		sb.add(String::fromInt32(d.hour, 10, 2));
		sb.addStatic(":", 1);
		sb.add(String::fromInt32(d.minute, 10, 2));
		sb.addStatic(":", 1);
		sb.add(String::fromInt32(d.second, 10, 2));
		return sb.merge();
	}

	String16 Time::getTimeString16(const TimeZone& zone) const noexcept
	{
		TimeComponents d;
		get(d, zone);
		StringBuffer16 sb;
		sb.add(String16::fromInt32(d.hour, 10, 2));
		{
			const sl_char16 ch = ':';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.minute, 10, 2));
		{
			const sl_char16 ch = ':';
			sb.addStatic(&ch, 1);
		}
		sb.add(String16::fromInt32(d.second, 10, 2));
		return sb.merge();
	}

	sl_bool Time::setString(const String& str, const TimeZone& zone) noexcept
	{
		if (parse(str, zone)) {
			return sl_true;
		} else {
			setZero();
			return sl_false;
		}
	}

	sl_bool Time::setString(const String16& str, const TimeZone& zone) noexcept
	{
		if (parse(str, zone)) {
			return sl_true;
		} else {
			setZero();
			return sl_false;
		}
	}

	sl_bool Time::setString(const AtomicString& str, const TimeZone& zone) noexcept
	{
		if (parse(str, zone)) {
			return sl_true;
		} else {
			setZero();
			return sl_false;
		}
	}

	sl_bool Time::setString(const AtomicString16& str, const TimeZone& zone) noexcept
	{
		if (parse(str, zone)) {
			return sl_true;
		} else {
			setZero();
			return sl_false;
		}
	}

	sl_bool Time::setString(const sl_char8* str, const TimeZone& zone) noexcept
	{
		if (parse(str, zone)) {
			return sl_true;
		} else {
			setZero();
			return sl_false;
		}
	}

	sl_bool Time::setString(const sl_char16* str, const TimeZone& zone) noexcept
	{
		if (parse(str, zone)) {
			return sl_true;
		} else {
			setZero();
			return sl_false;
		}
	}

	String Time::format(const String& fmt) const noexcept
	{
		return String::format(fmt, *this);
	}

	String Time::format(const AtomicString& fmt) const noexcept
	{
		return String::format(fmt, *this);
	}

	String Time::format(const String16& fmt) const noexcept
	{
		return String16::format(fmt, *this);
	}

	String Time::format(const AtomicString16& fmt) const noexcept
	{
		return String16::format(fmt, *this);
	}

	String Time::format(const sl_char8* fmt) const noexcept
	{
		return String::format(fmt, *this);
	}

	String Time::format(const sl_char16* fmt) const noexcept
	{
		return String16::format(fmt, *this);
	}


	template <class CT>
	static sl_reg _priv_Time_parseComponents(TimeComponents* comps, const CT* sz, sl_size i, sl_size n) noexcept
	{
		if (i >= n) {
			return SLIB_PARSE_ERROR;
		}
		sl_int32 YMDHMS[6];
		Base::resetMemory4(YMDHMS, 0, 6);
		sl_size index = 0;
		sl_size posParsed = i;
		while (i < n && index < 6) {
			if (sz[i] == 0) {
				break;
			}
			do {
				CT ch = sz[i];
				if (SLIB_CHAR_IS_SPACE_TAB(ch)) {
					i++;
				} else {
					break;
				}
			} while (i < n);
			if (i >= n) {
				break;
			}
			int value = 0;
			sl_bool flagNumber = sl_false;
			do {
				CT ch = sz[i];
				if (SLIB_CHAR_IS_DIGIT(ch)) {
					value = value * 10 + (ch - '0');
					flagNumber = sl_true;
					i++;
				} else {
					break;
				}
			} while (i < n);
			if (!flagNumber) {
				break;
			}
			posParsed = i;
			
			YMDHMS[index] = value;
			index++;
			
			if (i >= n) {
				break;
			}
			do {
				CT ch = sz[i];
				if (SLIB_CHAR_IS_SPACE_TAB(ch)) {
					i++;
				} else {
					break;
				}
			} while (i < n);
			if (i < n) {
				CT ch = sz[i];
				if (!SLIB_CHAR_IS_DIGIT(ch)) {
					if (ch != '/' && ch != '-' && ch != ':') {
						break;
					}
					if (ch == ':') {
						if (index < 3) {
							index = 3;
						}
					}
					i++;
				}
			} else {
				break;
			}
		}
		if (index > 0) {
			if (comps) {
				Base::zeroMemory(comps, sizeof(TimeComponents));
				comps->year = YMDHMS[0];
				comps->month = YMDHMS[1];
				comps->day = YMDHMS[2];
				comps->hour = YMDHMS[3];
				comps->minute = YMDHMS[4];
				comps->second = YMDHMS[5];
			}
			return posParsed;
		}
		return SLIB_PARSE_ERROR;
	}

	template <>
	sl_reg Parser<TimeComponents, sl_char8>::parse(TimeComponents* _out, const sl_char8 *sz, sl_size posBegin, sl_size posEnd) noexcept
	{
		return _priv_Time_parseComponents(_out, sz, posBegin, posEnd);
	}
	
	template <>
	sl_reg Parser<TimeComponents, sl_char16>::parse(TimeComponents* _out, const sl_char16 *sz, sl_size posBegin, sl_size posEnd) noexcept
	{
		return _priv_Time_parseComponents(_out, sz, posBegin, posEnd);
	}

	template <class CT>
	SLIB_INLINE static sl_reg _priv_Time_parse(Time* _out, const TimeZone& zone, const CT* sz, sl_size i, sl_size n) noexcept
	{
		TimeComponents comps;
		sl_reg ret = _priv_Time_parseComponents(&comps, sz, i, n);
		if (ret != SLIB_PARSE_ERROR) {
			if (_out) {
				_out->set(comps, zone);
			}
		}
		return ret;
	}

	template <>
	sl_reg Parser<Time, sl_char8>::parse(Time* _out, const sl_char8 *sz, sl_size posBegin, sl_size posEnd) noexcept
	{
		return _priv_Time_parse(_out, TimeZone::Local, sz, posBegin, posEnd);
	}

	template <>
	sl_reg Parser<Time, sl_char16>::parse(Time* _out, const sl_char16 *sz, sl_size posBegin, sl_size posEnd) noexcept
	{
		return _priv_Time_parse(_out, TimeZone::Local, sz, posBegin, posEnd);
	}

	template <>
	sl_reg Parser2<Time, sl_char8, TimeZone>::parse(Time* _out, const TimeZone& zone, const sl_char8 *sz, sl_size posBegin, sl_size posEnd) noexcept
	{
		return _priv_Time_parse(_out, zone, sz, posBegin, posEnd);
	}
	
	template <>
	sl_reg Parser2<Time, sl_char16, TimeZone>::parse(Time* _out, const TimeZone& zone, const sl_char16 *sz, sl_size posBegin, sl_size posEnd) noexcept
	{
		return _priv_Time_parse(_out, zone, sz, posBegin, posEnd);
	}

	Time& Time::operator=(const String& time) noexcept
	{
		setString(time);
		return *this;
	}

	Time& Time::operator=(const String16& time) noexcept
	{
		setString(time);
		return *this;
	}

	Time& Time::operator=(const AtomicString& time) noexcept
	{
		setString(time);
		return *this;
	}

	Time& Time::operator=(const AtomicString16& time) noexcept
	{
		setString(time);
		return *this;
	}

	Time& Time::operator=(const sl_char8* time) noexcept
	{
		setString(time);
		return *this;
	}

	Time& Time::operator=(const sl_char16* time) noexcept
	{
		setString(time);
		return *this;
	}

#if defined(SLIB_PLATFORM_IS_WINDOWS)
	sl_bool Time::_get(TimeComponents& output, sl_bool flagUTC) const noexcept
	{
		SYSTEMTIME st;
		sl_int64 n = (m_time + SLIB_INT64(11644473600000000)) * 10;  // Convert 1970 Based (time_t mode) to 1601 Based (FILETIME mode)
		if (flagUTC) {
			if (!(FileTimeToSystemTime((PFILETIME)&n, &st))) {
				return sl_false;
			}
		} else {
			SYSTEMTIME utc;
			if (!(FileTimeToSystemTime((PFILETIME)&n, &utc))) {
				return sl_false;
			}
			if (!(SystemTimeToTzSpecificLocalTime(NULL, &utc, &st))) {
				return sl_false;
			}
		}
		output.year = st.wYear;
		output.month = (sl_uint8)(st.wMonth);
		output.day = (sl_uint8)(st.wDay);
		output.dayOfWeek = (sl_uint8)(st.wDayOfWeek);
		output.hour = (sl_uint8)(st.wHour);
		output.minute = (sl_uint8)(st.wMinute);
		output.second = (sl_uint8)(st.wSecond);
		return sl_true;
	}

	sl_int64 Time::_set(int year, int month, int day, int hour, int minute, int second, sl_bool flagUTC) noexcept
	{
		SYSTEMTIME st;
		st.wYear = (WORD)year;
		st.wMonth = (WORD)month;
		st.wDay = (WORD)day;
		st.wDayOfWeek = 0;
		st.wHour = (WORD)hour;
		st.wMinute = (WORD)minute;
		st.wSecond = (WORD)second;
		st.wMilliseconds = 0;
		sl_int64 n = 0;
		if (flagUTC) {
			SystemTimeToFileTime(&st, (PFILETIME)&n);
		} else {
			SYSTEMTIME utc;
			TzSpecificLocalTimeToSystemTime(NULL, &st, &utc);
			SystemTimeToFileTime(&utc, (PFILETIME)&n);
		}
		return n / 10 - SLIB_INT64(11644473600000000);  // Convert 1601 Based (FILETIME mode) to 1970 Based (time_t mode)
	}

	void Time::_setNow() noexcept
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		sl_int64 n;
		SystemTimeToFileTime(&st, (PFILETIME)&n);
		m_time = n / 10 - SLIB_INT64(11644473600000000); // Convert 1601 Based (FILETIME mode) to 1970 Based (time_t mode)
	}
	
#elif defined(SLIB_PLATFORM_IS_UNIX)
	sl_bool Time::_get(TimeComponents& output, sl_bool flagUTC) const noexcept
	{
		time_t t = (time_t)(m_time / TIME_SECOND);
		if ((m_time % TIME_SECOND) < 0) {
			t -= 1;
		}
		tm v;
		if (flagUTC) {
			if (!(gmtime_r(&t, &v))) {
				return sl_false;
			}
		} else {
			if (!(localtime_r(&t, &v))) {
				return sl_false;
			}
		}
		output.year = v.tm_year + 1900;
		output.month = v.tm_mon + 1;
		output.day = v.tm_mday;
		output.dayOfWeek = v.tm_wday;
		output.hour = v.tm_hour;
		output.minute = v.tm_min;
		output.second = v.tm_sec;
		return sl_true;
	}
	
	sl_int64 Time::_set(int year, int month, int day, int hour, int minute, int second, sl_bool flagUTC) noexcept
	{
		tm v;
		Base::resetMemory(&v, 0, sizeof(tm));
		v.tm_year = year - 1900;
		v.tm_mon = month - 1;
		v.tm_mday = day;
		v.tm_hour = hour;
		v.tm_min = minute;
		v.tm_sec = second;
		v.tm_isdst = -1;
		time_t t;
		if (flagUTC) {
			t = timegm(&v);
		} else {
			t = mktime(&v);
		}
		return (sl_int64)(t) * 1000000;
	}

	void Time::_setNow() noexcept
	{
		sl_uint64 t;
		timeval tv;
		if (0 == gettimeofday(&tv, sl_null)) {
			t = tv.tv_sec;
			t *= TIME_SECOND;
			t += tv.tv_usec;
		} else {
			t = time(sl_null);
			t *= TIME_SECOND;
		}
		m_time = t;
	}
#endif

#if defined(SLIB_PLATFORM_IS_WIN32)
	sl_bool Time::_setToSystem() const noexcept
	{
		SYSTEMTIME st;
		sl_int64 n = (m_time + SLIB_INT64(11644473600000000)) * 10;  // Convert 1970 Based (time_t mode) to 1601 Based (FILETIME mode)
		if (!(FileTimeToSystemTime((PFILETIME)&n, &st))) {
			return sl_false;
		}
		return 0 != SetSystemTime(&st);
	}
#elif defined(SLIB_PLATFORM_IS_UNIX)
	sl_bool Time::_setToSystem() const noexcept
	{
		sl_int64 t = m_time;
		timeval tv;
		tv.tv_sec = (time_t)(t / TIME_SECOND);
		int m = (int)(t % TIME_SECOND);
		if (m < 0) {
			m += TIME_SECOND;
			tv.tv_sec -= 1;
		}
		tv.tv_usec = m;
		return 0 == settimeofday(&tv, sl_null);
	}
#else
	sl_bool Time::_setToSystem() const noexcept
	{
		return sl_false;
	}
#endif
	

	TimeCounter::TimeCounter() noexcept
	{
		reset();
	}

	TimeCounter::~TimeCounter() noexcept
	{
	}

	Time TimeCounter::getTime() const noexcept
	{
		return getTime(Time::now());
	}

	Time TimeCounter::getTime(const Time& current) const noexcept
	{
		Time last = m_timeLast;
		if (current > last) {
			return current - last + m_timeElapsed;
		} else {
			return m_timeElapsed;
		}
	}

	sl_uint64 TimeCounter::getElapsedMilliseconds() const noexcept
	{
		return getTime().getMillisecondsCount();
	}

	sl_uint64 TimeCounter::getElapsedMilliseconds(const Time& current) const noexcept
	{
		return getTime(current).getMillisecondsCount();
	}

	void TimeCounter::reset() noexcept
	{
		reset(Time::now());
	}

	void TimeCounter::reset(const Time& current) noexcept
	{
		m_timeLast = current;
		m_timeElapsed.setZero();
	}

	void TimeCounter::update() noexcept
	{
		update(Time::now());
	}

	void TimeCounter::update(const Time& current) noexcept
	{
		Time last = m_timeLast;
		if (current > last) {
			m_timeElapsed += (current - last);
		}
		m_timeLast = current;
	}


	TimeKeeper::TimeKeeper() noexcept
	 : m_flagStarted(sl_false), m_flagRunning(sl_false), m_timeLast(0), m_timeElapsed(0)
	{
	}

	TimeKeeper::~TimeKeeper() noexcept
	{
	}

	void TimeKeeper::start() noexcept
	{
		startAndSetTime(Time::zero(), Time::now());
	}

	void TimeKeeper::start(const Time& current) noexcept
	{
		startAndSetTime(Time::zero(), current);
	}

	void TimeKeeper::startAndSetTime(const Time& init) noexcept
	{
		startAndSetTime(init, Time::now());
	}

	void TimeKeeper::startAndSetTime(const Time& init, const Time& current) noexcept
	{
		SpinLocker lock(&m_lock);
		if (m_flagStarted) {
			return;
		}
		m_timeLast = current;
		m_timeElapsed = init;
		m_flagStarted = sl_true;
		m_flagRunning = sl_true;
	}

	void TimeKeeper::restart() noexcept
	{
		restartAndSetTime(Time::zero(), Time::now());
	}

	void TimeKeeper::restart(const Time& current) noexcept
	{
		restartAndSetTime(Time::zero(), current);
	}

	void TimeKeeper::restartAndSetTime(const Time& init) noexcept
	{
		restartAndSetTime(init, Time::now());
	}

	void TimeKeeper::restartAndSetTime(const Time& init, const Time& current) noexcept
	{
		SpinLocker lock(&m_lock);
		m_timeLast = current;
		m_timeElapsed = init;
		m_flagStarted = sl_true;
		m_flagRunning = sl_true;
	}

	void TimeKeeper::stop() noexcept
	{
		m_flagStarted = sl_false;
	}

	void TimeKeeper::resume() noexcept
	{
		resume(Time::now());
	}

	void TimeKeeper::resume(const Time& current) noexcept
	{
		SpinLocker lock(&m_lock);
		if (m_flagStarted) {
			if (!m_flagRunning) {
				m_timeLast = current;
				m_flagRunning = sl_true;
			}
		}
	}

	void TimeKeeper::pause() noexcept
	{
		pause(Time::now());
	}

	void TimeKeeper::pause(const Time& current) noexcept
	{
		SpinLocker lock(&m_lock);
		if (m_flagStarted) {
			if (m_flagRunning) {
				if (current > m_timeLast) {
					m_timeElapsed += (current - m_timeLast);
				}
				m_flagRunning = sl_false;
			}
		}
	}

	Time TimeKeeper::getTime() const noexcept
	{
		return getTime(Time::now());
	}

	Time TimeKeeper::getTime(const Time& current) const noexcept
	{
		SpinLocker lock(&m_lock);
		if (!m_flagStarted) {
			return Time::zero();
		}
		if (!m_flagRunning) {
			return m_timeElapsed;
		}
		return m_timeElapsed + (current - m_timeLast);
	}

	void TimeKeeper::setTime(const Time& time) noexcept
	{
		setTime(time, Time::now());
	}

	void TimeKeeper::setTime(const Time& time, const Time& current) noexcept
	{
		SpinLocker lock(&m_lock);
		if (m_flagStarted) {
			m_timeElapsed = time;
			m_timeLast = current;
		}
	}

	void TimeKeeper::update() noexcept
	{
		update(Time::now());
	}

	void TimeKeeper::update(const Time& current) noexcept
	{
		SpinLocker lock(&m_lock);
		if (m_flagStarted) {
			if (m_flagRunning) {
				sl_int64 add = (current - m_timeLast).toInt();
				if (add > 0) {
					m_timeElapsed += add;
				}
				m_timeLast = current;
			}
		}
	}

	sl_bool TimeKeeper::isStarted() const noexcept
	{
		return m_flagStarted;
	}

	sl_bool TimeKeeper::isStopped() const noexcept
	{
		return !m_flagStarted;
	}

	sl_bool TimeKeeper::isRunning() const noexcept
	{
		return m_flagStarted && m_flagRunning;
	}

	sl_bool TimeKeeper::isNotRunning() const noexcept
	{
		return !(m_flagStarted && m_flagRunning);
	}

	sl_bool TimeKeeper::isPaused() const noexcept
	{
		return m_flagStarted && !m_flagRunning;
	}

}
