/*
 * MessageThread.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: joe
 */

#include "MessageThread.h"
#include <iostream>

#include <QMutexLocker>

namespace mixpanel {
namespace details {

MessageThread::MessageThread()
    : QObject(0) {
	m_running = false;
    m_thread = new QThread();
    m_thread->setObjectName(QString("MixpanelMessageThread_InternalThread"));
    MessageWorker *worker = new MessageWorker();
    bool ok;
    // Hook our signals to the worker (for now, within our thread)
    ok = connect(this, SIGNAL(signalMessage(enum mixpanel_endpoint, QString)), worker, SLOT(message(enum mixpanel_endpoint, QString)));
    Q_ASSERT(ok);
    ok = connect(this, SIGNAL(signalFlush()), worker, SLOT(flush()));
    Q_ASSERT(ok);
    // Listen on thread.
    ok = connect(m_thread, SIGNAL(started()), this, SLOT(threadStarted()));
    Q_ASSERT(ok);
    ok = connect(m_thread, SIGNAL(finished()), this, SLOT(threadFinished()));
    Q_ASSERT(ok);
    ok = connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    Q_ASSERT(ok);
    ok = connect(m_thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
    worker->moveToThread(m_thread);
    m_thread->start();
}

MessageThread::~MessageThread() {}

void MessageThread::message(enum mixpanel_endpoint endpoint, QString message) {
    QMutexLocker lock(&m_running_mutex);
    if (! m_running) {
        m_waiting.append((struct waiting_message) { endpoint, message });
    } else {
        emit signalMessage(endpoint, message);
    }
}

void MessageThread::flush() {
    emit signalFlush();
}

void MessageThread::threadStarted() {
	QMutexLocker lock(&m_running_mutex);
    m_running = true;
    QList<struct waiting_message>::const_iterator i;
    for(i = m_waiting.constBegin(); i != m_waiting.constEnd(); i++) {
        emit signalMessage(i->endpoint, i->message);
    }
    m_waiting.clear();
    flush();
}

void MessageThread::threadFinished() {
    std::cerr << "Mixpanel Message Thread Finished\n";
}

} /* namespace details */
} /* namespace mixpanel */
