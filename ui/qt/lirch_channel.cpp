#include "ui/qt/lirch_channel.h"

// Makes a LirchChannel with the name channel_name managed by the UI *ui
LirchChannel::LirchChannel(const QString &channel_name, Ui::LirchQtInterface *ui) :
	name(channel_name),
	tab(new QWidget),
	tabs(ui->chatTabWidget),
	list(ui->chatUserList),
	menu(ui->menuViewTab),
	users(new QStandardItemModel(0, 1))
{
	// TODO Check to see if tab is duplicated
	int index = tabs->currentIndex();
	tabs->insertTab(index, tab, name);
	action = menu->addAction(name, this, SLOT(grab_tab_focus()));
	// Create a view and set its model
	browser = new QTextBrowser(tab);
	QPalette palette;
        QColor light_green(0xCC, 0xFF, 0xCC);
	palette.setColor(QPalette::Base, light_green);
	browser->setPalette(palette);
	document = new QTextDocument(browser);
	cursor = new QTextCursor(document);
	browser->setDocument(document);
	// Set up the layout with this view
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
	layout->addWidget(browser);
	tab->setLayout(layout);
	// Select the new window
	action->trigger();
	// TODO this should happen, but doesn't:
	grab_user_list();
}

LirchChannel::~LirchChannel() {
	// Remove the action/tab pair
	menu->removeAction(action);
	int index = tabs->indexOf(tab);
	if (index != -1) {
		tabs->removeTab(index);
	}
	// This triggers deletion of browser, document, and cursor
	delete tab;
	// This triggers deletion of users (new tab should have grabbed its user list)
	delete users;
}

void LirchChannel::show_message(const DisplayedMessage &message, bool show_timestamp) {
	if (show_timestamp) {
		cursor->insertText("[" + message.timestamp + "] ");
	}
	cursor->insertHtml(message.text);
	browser->ensureCursorVisible();
	// TODO make this insert between lines
	static QTextBlockFormat block_format;
	cursor->insertBlock(block_format);
}

void LirchChannel::update_users(const QSet<QString> &new_users) {
	users->clear();
	for (auto &new_user : new_users) {
		users->appendRow(new QStandardItem(new_user));
	}
}

// TODO make message reception do something to unfocused tabs, reset on focus
void LirchChannel::add_message(const QString& text, bool show_timestamp, bool ignore_message) {
	// Package the data so that we have it for reloads
	QString timestamp = QTime::currentTime().toString();
	DisplayedMessage message(text, timestamp, ignore_message);
	messages.append(message);
	if (!ignore_message) {
		show_message(message, show_timestamp);
	}
}

void LirchChannel::persist(QFile &file) const {
	emit persisting();
	// Persist the display messages to file using UTF-8
	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	// Report progress based on blocks
	int num_blocks = document->blockCount();
	QTextBlock block = document->begin();
	for (int i = 0; i < num_blocks; ++i) {
		emit progress(i * 100 / num_blocks);
		// Write the block and advance
		endl(stream << block.text());
		block = block.next();
	}
	emit progress(100);
	// TODO Insert wait to make sure window works?
	emit persisted();
}

void LirchChannel::grab_tab_focus() const {
	int index = tabs->indexOf(tab);
	if (index != -1) {
		tabs->setCurrentIndex(index);
	}
}

void LirchChannel::grab_user_list() const {
	list->setModel(users);
}

// TODO make ignore list per-channel (need to edit add_message)
void LirchChannel::reload_messages(bool show_timestamps, bool show_ignored) {
	document->clear();
	for (auto &message : messages) {
		if (!message.ignored || show_ignored) {
			show_message(message, show_timestamps);
		}
	}
}

