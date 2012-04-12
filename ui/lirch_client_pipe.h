#ifndef LIRCH_CLIENT_PIPE_H
#define LIRCH_CLIENT_PIPE_H

#include <QObject>
#include <QString>

#include "core/message_view.h"
#include "plugins/display_messages.h"

class LirchClientPipe : public QObject {
    Q_OBJECT
    // BEFORE refers to the period of time prior to open()
    // DURING, prior to close(), and AFTER, thereafter
    enum class State { BEFORE, DURING, AFTER };
public:
    explicit LirchClientPipe();
    virtual ~LirchClientPipe();
    // Used for querying the status on either end
    bool ready() const;
    // Called by the client when sending messages
    void send(const message &);
    // A client is notified when these are called
    void display(const display_message &);
    void open(plugin_pipe &pipe, const QString &name);
    void close(const QString &reason = "unknown reason");
private:
    // All client pipes have a mode, see above
    State client_state;
    // All client pipes wrap a plugin pipe for a specific (named) plugin
    QString client_name;
    plugin_pipe client_pipe;
signals:
    // For alerting the UI when to start/stop [show()/close()]
    void run(LirchClientPipe *);
    void shutdown(const QString &);
    // For alerting the UI of an inbound message
    void alert(const QString &, const QString &);
};

#endif // LIRCH_CLIENT_PIPE_H