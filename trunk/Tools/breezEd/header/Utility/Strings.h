#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include "breezEd.h"
#include <lean/strings/string_traits.h>
#include <QtCore/QString>
#include <QtCore/QCoreApplication>

namespace lean
{
namespace strings
{

template <>
struct string_traits<QString>
{
	/// String type.
	typedef QString string_type;
	/// Character type,
	typedef string_type::value_type value_type;
	/// Iterator type.
	typedef string_type::iterator iterator;
	/// Const iterator type.
	typedef string_type::const_iterator const_iterator;
	/// Size type.
	typedef unsigned int size_type;
	
	/// Constructs a string from the given range.
	template <class Iterator>
	static LEAN_INLINE string_type construct(Iterator begin, Iterator end)
	{
		return string_type(begin, end - begin);
	}
	/// Assigns the given range to the given string.
	template <class Iterator>
	static LEAN_INLINE void assign(string_type &str, Iterator begin, Iterator end)
	{
		str = string_type(begin, end - begin);
	}

	/// Resizes the given string.
	static LEAN_INLINE void resize(string_type &str, size_type size)
	{
		str.resize( static_cast<int>(size) );
	}
	/// Reserves the given amount of space.
	static LEAN_INLINE void reserve(string_type &str, size_type size)
	{
		str.reserve( static_cast<int>(size) );
	}

	/// Erases the given range of characters.
	static LEAN_INLINE void erase(string_type &str, iterator begin, iterator end)
	{
		str.remove(begin - str.begin(), end - begin);
	}

	/// Checks if the given string is empty.
	static LEAN_INLINE bool empty(const string_type &str)
	{
		return str.isEmpty();
	}
	/// Gets the size of the given string.
	static LEAN_INLINE size_type size(const string_type &str)
	{
		return str.size();
	}

	/// Gets the beginning of the given range.
	static LEAN_INLINE iterator begin(string_type &str)
	{
		return str.begin();
	}
	/// Gets the beginning of the given range.
	static LEAN_INLINE const_iterator begin(const string_type &str)
	{
		return str.begin();
	}
	/// Gets the end of the given range.
	static LEAN_INLINE iterator end(string_type &str)
	{
		return str.end();
	}
	/// Gets the end of the given range.
	static LEAN_INLINE const_iterator end(const string_type &str)
	{
		return str.end();
	}
};

} // namespace
} // namespace

/// Builds a QString from the given nullterminated character range.
inline QString toQt(const lean::utf8_ntri &str)
{
	return QString::fromUtf8(str.c_str(), static_cast<int>(str.size()));
}

/// Builds an utf8_string from the given QString.
inline lean::utf8_ntr toUtf8Range(const QString &str)
{
	const QByteArray &bytes = str.toUtf8();
	return lean::utf8_ntr(bytes.data(), bytes.data() + bytes.size());
}

/// Builds an utf8_string from the given QString.
inline lean::utf8_string toUtf8(const QString &str)
{
	const QByteArray &bytes = str.toUtf8();
	return lean::utf8_string(bytes.data(), bytes.size());
}

/// Makes sure the given string is not empty.
inline QString makeName(const QString &name)
{
	return (!name.isEmpty())
		? name
		: QCoreApplication::translate("MakeName", "<unnamed>");
}

#endif