// TODO use QDebug for output?
#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#include <QApplication>
#include <QObject>
#include <QString>

#include "lirch_constants.h"
#include "core/core_messages.h"
#include "ui/lirch_client_pipe.h"
#include "ui/qt/lirch_qt_interface.h"

// Necessary for communication with lirch-qt
LirchClientPipe mediator;

// Declared in core.cpp
void run_core(const vector<message> &vm);
extern bool verbose;

// Necessary for parsing arguments
#include <tclap/CmdLine.h>

static struct lirch_options {
  bool list_preloads, no_preloads, verbose;
  vector<string> plugins;
} session;

void parse_args(int argc, char *argv[]) {
	try {
		// Init the options object
		QString id = QObject::tr("%1 %2 core (%3)").arg(
						LIRCH_PRODUCT_NAME,
						LIRCH_VERSION_STRING,
						LIRCH_BUILD_HASH);
		TCLAP::CmdLine options(id.toStdString(), ' ', LIRCH_VERSION_STRING);
		// Specify switches TODO functionalize? tr()?
		TCLAP::SwitchArg verboseSwitch("v", "verbose",
			"Enable loud output (all messages)", options, false);
		TCLAP::SwitchArg noPreloadsSwitch("n", "no_preloads",
			"Don't load the preloaded plugins", options, false);
		TCLAP::SwitchArg listPreloadsSwitch("l", "list_preloads",
			"List the preloaded plugins", options, false);
		// We're allowed exactly one of these:
		TCLAP::UnlabeledMultiArg<string> plugin_pairs("plugins",
			"Bare strings, in pairs like: antenna lib/libantenna.so", false,
			"plugin_name plugin_file", "pair<string, string>");
                options.add(plugin_pairs);
		// Parse all the options, and fill in session
		options.parse(argc, argv);
		session.verbose = verboseSwitch.getValue();
		session.no_preloads = noPreloadsSwitch.getValue();
		session.list_preloads = listPreloadsSwitch.getValue();
		session.plugins = plugin_pairs.getValue();
	} catch (TCLAP::ArgException &e) {
		// TODO better error messages?
		cerr << e.error() << endl;
	}
}

int main(int argc, char *argv[])
{
	// TODO does this need to happen right away? Guess so.
	QApplication qlirch(argc, argv);
	// TODO sometimes being necessary to avoid string conversion issues?
	setlocale(LC_NUMERIC, "C");
	// The window is constructed here, show()'n later (see ui/qt)
	LirchQtInterface main_window;
        // A small intermediate object is used to mediate
        QObject::connect(&mediator,    SIGNAL(alert(QString, QString)),
                         &main_window, SLOT(display(QString, QString)));
        QObject::connect(&mediator,    SIGNAL(shutdown(QString)),
                         &main_window, SLOT(die(QString)));
        QObject::connect(&mediator,    SIGNAL(run(LirchClientPipe *)),
                         &main_window, SLOT(use(LirchClientPipe *)));
        QObject::connect(&qlirch,      SIGNAL(aboutToQuit()),
                         &mediator,    SLOT(join()));

	vector<pair<string, string>> plugins;
	// Prepare the list of plugins specified in build (see lirch_constants.h)
	for (auto &p : preloads)
	{
		plugins.push_back(make_pair<string, string>(p.name, p.filename));
	}

	// Get the remaining arguments from the command line
	parse_args(argc, argv);
	verbose = session.verbose;
	QString error_msg;

	// Handle a request to display these
	if (session.list_preloads) {
		error_msg = QObject::tr("Plugins configured to preload: ");
		if (session.no_preloads) {
			error_msg += QObject::tr("(Note: --no_preloads flag was specified.)");
		}
		cerr << error_msg.toStdString() << endl;
		for (auto &p : plugins) {
			cerr << p.first << ": " << p.second << endl;
		}
	}

	// Honor a request to ignore these
	if (session.no_preloads) {
		plugins.clear();
	}

	// Add in plugins specified by the command line
	vector<string>::const_iterator pit, pitend;
	pit = session.plugins.begin();
	pitend = session.plugins.end();
	pair<string, string> p;
	error_msg = QObject::tr("[WARNING] ignoring unpaired plugin argument '%1'");
	while (pit != pitend) {
		p.first = *pit; ++pit;
		if (pit == pitend) {
			error_msg = error_msg.arg(QString::fromStdString(p.first));
			cerr << error_msg.toStdString() << endl;
			break;
		}
		p.second = *pit; ++pit;
		plugins.push_back(p);
	}

	// Run the core in a separate thread
	vector<message> add_messages;
	for (auto &p : plugins) {
		add_messages.push_back(plugin_adder::create(p.first, p.second));
	}
	thread core_thread(run_core, add_messages);
	// The mediator needs to know which thread to join
	mediator.load(&core_thread);

	// When this event loop terminates, so will we
	return qlirch.exec();
}