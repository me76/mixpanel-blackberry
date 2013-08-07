/*
 * MessageThread.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: joe
 */

#include "MessageThread.h"
#include <iostream>

namespace mixpanel {
namespace details {

MessageThread::MessageThread()
	: QObject(0) {
	m_thread = new QThread();
	m_thread->setObjectName(QString("MixpanelMessageThread_InternalThread"));
	m_worker = new MessageWorker();

	bool ok;
	// Hook our signals to the worker (for now, within our thread)
	ok = connect(this, SIGNAL(signalMessage(enum mixpanel_endpoint, QString)), m_worker, SLOT(message(enum mixpanel_endpoint, QString)));
	Q_ASSERT(ok);
	ok = connect(this, SIGNAL(signalFlush()), m_worker, SLOT(flush()));
	Q_ASSERT(ok);

	// Listen on thread.
	ok = connect(m_thread, SIGNAL(started()), this, SLOT(threadStarted()));
	Q_ASSERT(ok);
	ok = connect(m_thread, SIGNAL(finished()), this, SLOT(threadFinished()));
	Q_ASSERT(ok);
	ok = connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
	Q_ASSERT(ok);
	Q_UNUSED(ok);

	m_thread->start();
}

MessageThread::~MessageThread() {
	if (NULL != m_worker) {
		// Thread never got started.
		delete m_worker;
	}
}

void MessageThread::message(enum mixpanel_endpoint endpoint, QString message) {
	emit signalMessage(endpoint, message);
}

void MessageThread::flush() {
	emit signalFlush();
}

void MessageThread::threadStarted() {
	// Wait to move the worker into our thread so early tracking signals aren't lost.
	// TODO WHAT IF THREAD IS ALREADY DEAD?
	m_worker->moveToThread(m_thread);
	// TODO RACE CONDITION HERE
	bool ok = connect(m_thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
	Q_ASSERT(ok);
	Q_UNUSED(ok);
	m_worker = NULL; // Other thread owns this now.
	std::cerr << "Mixpanel Message Thread Started\n";
}

void MessageThread::threadFinished() {
	std::cerr << "Mixpanel Message Thread Finished\n";
}

} /* namespace details */
} /* namespace mixpanel */
