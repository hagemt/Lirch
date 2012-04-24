// TODO (not in order of importance)
// 1) Ugh, fix the layout somehow?
// 2) Setup Wizard
//    a) Nick menu
//    b) Ignore/Block menu
// 3) Tab Management (use QTabWidget or QWidget container?)
//    !) Representation of each chatView
//    ?) Default presentation
//    a) Integration with Menus
//    b) Integration with User Lists
// 4) Message Pipe interactions
//    a) Channel creation
//    b) Polling for participants

#include "ui/qt/lirch_qt_interface.h"
#include "ui/qt/ui_lirch_qt_interface.h"
#include "ui/qt/lirch_qlineedit_dialog.h"
#include "ui/qt/ui_lirch_qlineedit_dialog.h"

// QT UI

LirchQtInterface::LirchQtInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LirchQtInterface),
    settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME)
{
    // Initialize the UI
    ui->setupUi(this);
    // Add a variety of UI enhancements (select on focus and quit action)
    ui->msgTextBox->installEventFilter(this);
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    // client_pipe facilitates communication with the core
    client_pipe = nullptr;

    // Load up the settings and kick things off
    loadSettings();
}

LirchQtInterface::~LirchQtInterface()
{
    // Save settings on destruction
    saveSettings();
    delete ui;
}

// settings-related (IMPORTANT: always edit these functions as a pair)

void LirchQtInterface::loadSettings()
{
    // Make sure to document any changes in the settings schema (change below and on wiki)
    settings.beginGroup("UserData");
    nick = settings.value("nick", default_nick).value<QString>();
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    resize(settings.value("size", QSize(640, 480)).toSize());
    move(settings.value("position", QPoint(100, 100)).toPoint());
    settings.endGroup();
    settings.beginGroup("ChatView");
    settings.beginGroup("Messages");
    show_ignored_messages = settings.value("show_ignored_messages", false).value<bool>();
    show_message_timestamps = settings.value("show_message_timestamps", true).value<bool>();
    settings.endGroup();
    settings.endGroup();
}

void LirchQtInterface::saveSettings()
{
    // Make sure to document any changes in the settings schema (change above and on wiki)
    settings.beginGroup("UserData");
    settings.setValue("nick", default_nick);
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    settings.setValue("size", size());
    settings.setValue("position", pos());
    settings.endGroup();
    settings.beginGroup("ChatView");
    settings.beginGroup("Messages");
    settings.setValue("show_ignored_messages", show_ignored_messages);
    settings.setValue("show_message_timestamps", show_message_timestamps);
    settings.endGroup();
    settings.endGroup();
}

// event-related

void LirchQtInterface::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


bool LirchQtInterface::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->msgTextBox) {
        // Filter FocusIn events on the text box to select all text therein
        if (event->type() == QEvent::FocusIn) {
            // FIXME there has to be a better way to do this
            QTimer::singleShot(0, object, SLOT(selectAll()));
            return true;
        }
        return false;
    }
    return QMainWindow::eventFilter(object, event);
}

void LirchQtInterface::on_msgSendButton_clicked()
{
    // Get the text to send
    QString text = ui->msgTextBox->text();
    // Ignore empty case
    if (text.isEmpty()) {
        return;
    }

    // The core will pass this raw edict to the meatgrinder
    client_pipe->send(raw_edict_message::create(text, LIRCH_DEFAULT_CHANNEL));

    // TODO here is the failure condition, when does this happen?
    // QMessageBox::warning(this,
    //                     tr("Unimplemented Feature"),
    //                     tr("'%1' could not be sent.").arg(text),
    //                     QMessageBox::Ok);

    // Don't forget to clear the text from the box
    ui->msgTextBox->clear();
}

// FILE MENU

void LirchQtInterface::on_actionConnect_triggered(bool checked)
{
    // TODO actual delegation to core (core forwards to antenna)
    QString timestamp = QTime::currentTime().toString();
    if (checked) {
        ui->chatViewArea->append("At [" + timestamp + "]: /join'd #default");
    } else {
        ui->chatViewArea->append("At [" + timestamp + "]: /part'd #default");
    }
}

// new-related

void LirchQtInterface::on_actionNewChannel_triggered()
{
    alert_user(tr("The %1 feature is forthcoming.").arg("New > Private Channel"));
}

void LirchQtInterface::on_actionNewTransfer_triggered()
{
    alert_user(tr("The %1 feature is forthcoming.").arg("New > File Transfer"));
}

// log-related

void LirchQtInterface::on_actionSaveLog_triggered()
{
    QMessageBox::information(this,
                             tr("Confirmation"),
                             tr("Log saved: %1/default.log").arg(LIRCH_DEFAULT_LOG_DIR));
}

void LirchQtInterface::on_actionOpenLog_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Log File"), "./", tr("Logs (*.log)"));
    // FIXME for now, just load the log into the tab
    ui->chatLogArea->clear();
    ui->chatLogArea->append(filename);
    int index = ui->chatTabWidget->indexOf(ui->logChannelTab);
    if (index != -1) {
        ui->chatTabWidget->setCurrentIndex(index);
    }
}

// EDIT MENU

void LirchQtInterface::on_actionEditNick_triggered()
{
    LirchQLineEditDialog nick_dialog;
    connect(&nick_dialog, SIGNAL(submit(QString, bool)), this, SLOT(nick_changed(QString, bool)));
    nick_dialog.exec();
}

void LirchQtInterface::on_actionEditIgnored_triggered()
{
    LirchQLineEditDialog ignore_dialog;
    connect(&ignore_dialog, SIGNAL(submit(QString, bool)), this, SLOT(ignore_changed(QString, bool)));
    ignore_dialog.exec();
}

// VIEW MENU

// TODO is there some built-in stuff for things like these?

void LirchQtInterface::on_actionViewUserList_toggled(bool checked)
{
    if (checked) {
        ui->chatUserList->show();
    } else {
        ui->chatUserList->hide();
    }
}

void LirchQtInterface::on_actionViewSendButton_toggled(bool checked)
{
    if (checked) {
        ui->msgSendButton->show();
    } else {
        ui->msgSendButton->hide();
    }
}

void LirchQtInterface::on_actionViewTimestamps_toggled(bool checked)
{
    show_message_timestamps = checked;
}

void LirchQtInterface::on_actionViewIgnored_toggled(bool checked)
{
    show_ignored_messages = checked;
}

// tab-related

void LirchQtInterface::on_actionViewDefault_triggered()
{
    int index = ui->chatTabWidget->indexOf(ui->defaultChannelTab);
    if (index != -1) {
        ui->chatTabWidget->setCurrentIndex(index);
    }
}

void LirchQtInterface::on_actionViewTransfers_triggered()
{
	alert_user(tr("The %1 feature is forthcoming.").arg("View > File Tranfers"));
}

// ABOUT MENU

void LirchQtInterface::on_actionWizard_triggered()
{
	alert_user(tr("The %1 feature is forthcoming.").arg("Help > Setup Wizard"));
}

void LirchQtInterface::on_actionAbout_triggered()
{
	QMessageBox about_box;
	about_box.setIcon(QMessageBox::Information);
	about_box.setWindowTitle(tr("About %1 %2").arg(
		LIRCH_PRODUCT_NAME,
		LIRCH_VERSION_STRING));
	about_box.setText(tr("%1 %2 (%3) is Copyright (c) %4, %5 (see README)").arg(
		LIRCH_PRODUCT_NAME,
		LIRCH_VERSION_STRING,
		LIRCH_BUILD_HASH,
		LIRCH_COPYRIGHT_YEAR,
		LIRCH_COMPANY_NAME));
	about_box.setInformativeText("Do you want to report a bug?");
	about_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	about_box.setDefaultButton(QMessageBox::No);
	if (about_box.exec() == QMessageBox::Yes) {
		QString date, time, title, body, url;
		date = QDate::currentDate().toString();
		time = QTime::currentTime().toString();
		title = tr("Auto-Generated Bug Report [%1 at %2]").arg(date, time);
		body = tr("I am experiencing the following issue with %1 %2 (%3):").arg(
			LIRCH_PRODUCT_NAME,
			LIRCH_VERSION_STRING,
			LIRCH_BUILD_HASH);
		url = tr("https://github.com/m42a/Lirch/issues/new?title=%1&body=%2");
		QDesktopServices::openUrl(QUrl(url.arg(title, body)));
	}
}

// INTERNAL SLOTS

// TODO does this happen on any show? if so, don't connect
void LirchQtInterface::showEvent(QShowEvent *e)
{
	// TODO first-time QWizard to determine if these happen:
	// The first tab is always the default channel
	ui->actionViewDefault->trigger();
	// Connect to it
	ui->actionConnect->setChecked(true);
	// TODO get the textbox to focus on any Show
	// ui->msgTextBox->setFocus(true);
	// Propagate
	e->ignore();
}

void LirchQtInterface::closeEvent(QCloseEvent *e)
{
	// Confirm the close (potentially ignore)
	if (client_pipe == nullptr || !client_pipe->ready() ||
		QMessageBox::question(this,
			tr("Close Prompt"),
			tr("Are you sure you want to quit %1?").arg(LIRCH_PRODUCT_NAME),
			QMessageBox::Yes,
			QMessageBox::No,
			QMessageBox::NoButton) == QMessageBox::Yes) {
		e->accept();
	} else {
		e->ignore();
	}
}

void LirchQtInterface::alert_user(QString msg)
{
	// Empty alerts trigger fatal errors to encourage proper alerts
	if (msg.isEmpty()) {
		die(tr("Empty user alert message."));
	} else {
		QMessageBox::information(this, tr("Alert"), msg);
	}
}

void LirchQtInterface::use(LirchClientPipe *pipe)
{
    if (pipe != nullptr && pipe->ready()) {
        client_pipe = pipe;
        this->show();
    } else {
        QMessageBox::information(this,
                                 tr("Error"),
                                 tr("Client pipe failed to connect."));
    }
}

void LirchQtInterface::die(QString msg)
{
    QMessageBox::information(this,
                             tr("Fatal Error"),
                             tr("Details: '%1'").arg(msg));
    this->close();
}

void LirchQtInterface::display(QString channel, QString contents) {
    QString timestamp = "[" + QTime::currentTime().toString() + "]";
    // FIXME Something funny is going on here
    if (channel.isEmpty()) {
        ui->chatViewArea->append("At " + timestamp + ": /recv'd mangled message");
        return;
    }
    if (channel != tr(LIRCH_DEFAULT_CHANNEL)) {
        ui->chatViewArea->append("At " + timestamp + ": /recv'd message on channel: " + channel);
    }

    // Show the message in the view
    if (show_message_timestamps) {
        ui->chatViewArea->append(timestamp + contents);
    } else {
        ui->chatViewArea->append(contents);
    }
}

void LirchQtInterface::nick_changed(QString new_nick, bool permanent)
{
    // TODO delegate to core (userlist needs to approve)
    nick = new_nick;
    if (permanent) {
        default_nick = new_nick;
    }
    // TODO make these actually send
    display(tr("internal"), tr("/nick %1").arg(nick));
}

void LirchQtInterface::ignore_changed(QString new_ignore, bool block)
{
    QString status = "dummy";
    // TODO delegate to core (antenna will block/ignore)
    if (block) {
        
    }
    // TODO make these actually send
    display(tr("internal"), tr("/ignore %1 (%2)").arg(new_ignore, status));
}

