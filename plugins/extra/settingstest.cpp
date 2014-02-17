#include <string>

#include <QSettings>

#include "lirch/core/required_messages.h"

#include "lirch/plugins/lirch_plugin.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "The Addams Family", "Lirch");
	p.write(done_message::create(name));
}
