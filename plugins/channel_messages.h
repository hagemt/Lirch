#ifndef CHANNEL_MESSAGES_H_
#define CHANNEL_MESSAGES_H_

#include <QStringList>


class set_channel_message : public message_data

{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new set_channel_message(*this));}
	static message create(const QString &ch) {return message_create("set_channel", new set_channel_message(ch));}

	set_channel_message(const QString &ch) : channel(ch) {}

	QString channel;
};

class leave_channel_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new leave_channel_message(*this));}
	static message create(const QString &ch) {return message_create("leave_channel", new leave_channel_message(ch));}

	leave_channel_message(const QString &ch) : channel(ch) {}

	QString channel;
};

class query_channel : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new query_channel(*this));}
	static message create() {return message_create("query_channel", new query_channel());}

	query_channel() {}
};

class channel_list : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new channel_list(*this));}
	static message create(const QStringList &ch) {return message_create("channel_list", new channel_list(ch));}

	channel_list(const QStringList &ch) : channels(ch) {}

	QStringList channels;
};
#endif
