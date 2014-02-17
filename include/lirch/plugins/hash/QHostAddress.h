#ifndef __LIRCH_HASH_QHOSTADDRESS_H__
#define __LIRCH_HASH_QHOSTADDRESS_H__

#include <QHostAddress>

#include "QString.h"

namespace std {

template <>
struct hash<QHostAddress> {
	size_t operator()(const QHostAddress &addr) const {
		return delegate(addr.toString());
	}
private:
	static hash<QString> delegate;
};

} // namespace std

#endif // __LIRCH_HASH_QHOSTADDRESS_H__
