#include "lirch_qt_interface.h"
#include "ui_lirch_qt_interface.h"

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

LirchQtInterface::LirchQtInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LirchQtInterface),
    settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, "Lirch")
{
    ui->setupUi(this);
    // Add a variety of enhancements
    ui->msgTextBox->installEventFilter(this);
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    // Load up the
    loadSettings();
}

LirchQtInterface::~LirchQtInterface()
{
    // Save settings on destruction, right?
    saveSettings();
    delete ui;
}

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

void LirchQtInterface::loadSettings()
{
    settings.beginGroup("UserData");
    // TODO default nick?
    nick = settings.value("nick", "Spartacus").value<QString>();
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    resize(settings.value("size", QSize(640, 480)).toSize());
    move(settings.value("position", QPoint(100, 100)).toPoint());
    settings.beginGroup("ChatLog");
    settings.beginGroup("Messages");
    show_timestamps = settings.value("show_timestamps", true).value<bool>();
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();
}

void LirchQtInterface::saveSettings()
{
    // TODO schema for settings
    settings.beginGroup("UserData");
    settings.setValue("nick", nick);
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    settings.setValue("size", size());
    settings.setValue("position", pos());
    settings.beginGroup("ChatLog");
    settings.beginGroup("Messages");
    settings.setValue("show_timestamps", show_timestamps);
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();
}

void LirchQtInterface::on_actionConnect_triggered(bool checked)
{
    // TODO actual delegation
    if (checked) {
        QMessageBox::information(this,
                                 tr("Alert"),
                                 tr("You connected!"));
    } else {
        QMessageBox::information(this,
                                 tr("Alert"),
                                 tr("You disconnected!"));
    }
}

bool LirchQtInterface::eventFilter(QObject *object, QEvent *event)
{
    // Filter FocusIn events on the text box to select all text therein
    if (event->type() == QEvent::FocusIn) {
        if (object == ui->msgTextBox) {
            // TODO there has to be a better way to do this
            QTimer::singleShot(0, object, SLOT(selectAll()));
            return false;
        }
    }
    return QObject::eventFilter(object, event);
}

void LirchQtInterface::on_msgSendButton_clicked()
{
    // All text is prefixed with the nick
    QString prefix = "<" + nick + ">";
    // And potentially a timestamp
    if (show_timestamps) {
        prefix += " [" + QTime::currentTime().toString() + "]";
    }
    // Get the text to send
    QString msg = ui->msgTextBox->text();
    // Ignore empty case
    if (msg.isEmpty()) {
        return;
    }
    // TODO wrap text in message and delegate
    ui->chatLogArea->append(prefix + ": " + msg);
    QMessageBox::warning(this,
                         tr("Unimplemented Feature"),
                         tr("'%1' could not be sent.").arg(msg),
                         QMessageBox::Ok);
    // Don't forget to clear the text from the box
    ui->msgTextBox->clear();
}

void LirchQtInterface::on_actionAbout_triggered()
{
    // TODO does this need to be its own widget?
    QMessageBox::information(this,
                             tr("About Lirch %1").arg(LIRCH_VERSION_STRING),
                             tr("Lirch %1 (%2) is Copyright (c) %3, %4 (see README)").arg(
                                     LIRCH_VERSION_STRING,
                                     LIRCH_BUILD_HASH,
                                     LIRCH_COPYRIGHT_YEAR,
                                     LIRCH_COMPANY_NAME));
}

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
    show_timestamps = checked;
}

void LirchQtInterface::on_actionViewIgnored_toggled(bool checked)
{
    show_ignored_messages = checked;
}
