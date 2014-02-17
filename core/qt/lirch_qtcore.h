#ifndef __LIRCH_CORE_QT__
#define __LIRCH_CORE_QT__

#include <iostream>
#include <thread>
#include <vector>

#include <QCoreApplication>
#include <QObject>

#include "core/message.h"

void run_core(const std::vector<message> &);

//namespace lirch {
//namespace core {

// Can actually manage more than one client on single core?
class lirch_core : public QObject {
    Q_OBJECT;
	std::thread *core_thread;
    std::vector<message> messages;

public:
    constexpr static int EXEC_FAILURE = -1;
    lirch_core(const std::vector<message> &pp) : core_thread(nullptr) {
		// FIXME(teh): could pass app and handle connect here?
		for (const message &m : pp) {
            //std::cerr << "[DEBUG] " << m << " (core invocation)" << std::endl;
			messages.push_back(m);
		}
	}

	virtual ~lirch_core() {
		if (core_thread) {
            //std::cerr << "[WARNING] core still in use (will detach)" << std::endl;
			core_thread->detach();
		}
	}

    int exec(QCoreApplication &app) {
		if (core_thread) return EXEC_FAILURE;
		core_thread = new std::thread([this, messages](){
			run_core(messages);
			// TODO(teh): check if app has quit?
			this->quit();
		});
		return app.exec();
	}

public slots:
	void quit() {
		if (core_thread) {
			emit aboutToQuit();
			core_thread->join();
			delete core_thread;
			core_thread = nullptr;
		}
	}

signals:
	void aboutToQuit();
};

//} // namespace core
//} // namespace lirch

#endif // __LIRCH_CORE_QT__
