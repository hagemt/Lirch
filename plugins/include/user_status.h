#ifndef __USER_STATUS_H__
#define __USER_STATUS_H__

#include <ctime>
#include <unordered_set>

#include <QString>
#include <QHostAddress>

#include "hash/QString.h"

struct user_status
{
	QString nick;
	QHostAddress ip;
	time_t lastseen;
	std::unordered_set<QString> channels;
};

#endif // __USER_STATUS_H__
