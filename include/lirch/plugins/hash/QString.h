#ifndef __LIRCH_HASH_QSTRING_H__
#define __LIRCH_HASH_QSTRING_H__

#include <functional>
#include <string>

#include <QString>

namespace std {

template <>
struct hash<QString>
{
	size_t operator()(const QString &str) const {
		return delegate(str.toStdString());
	}
private:
	static hash<string> delegate;
};

} // namespace std

#endif // __LIRCH_HASH_QSTRING_H__
