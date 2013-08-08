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
    : QObject(0), m_thread(), m_worker(), m_waiting(), m_running(false), m_running_mutex() {
    m_thread.setObjectName(QString("MixpanelMessageThread_InternalThread"));
    m_worker.moveToThread(&m_thread);
    bool ok;
    ok = connect(this, SIGNAL(signalMessage(enum mixpanel_endpoint, QString)), &m_worker, SLOT(message(enum mixpanel_endpoint, QString)));
    Q_ASSERT(ok);
    ok = connect(this, SIGNAL(signalFlush()), &m_worker, SLOT(flush()));
    Q_ASSERT(ok);
    // Listen on thread.
    ok = connect(&m_thread, SIGNAL(started()), this, SLOT(threadStarted()));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
    m_thread.start();
}

MessageThread::~MessageThread() {}

void MessageThread::message(enum mixpanel_endpoint endpoint, const QString &message) {
    QMutexLocker lock(&m_running_mutex);
    if (m_running) {
        emit signalMessage(endpoint, message);
    } else {
        m_waiting.append(std::pair<enum mixpanel_endpoint, QString>(endpoint, message));
    }
}

void MessageThread::flush() {
    QMutexLocker lock(&m_running_mutex);
    if (m_running) {
        emit signalFlush();
    }
}

void MessageThread::threadStarted() {
    QMutexLocker lock(&m_running_mutex);
    m_running = true;
    QTimer::singleShot(0, this, SLOT(pushWaiting())); // TODO doesn't work
}

void MessageThread::pushWaiting() {
    QList< std::pair<enum mixpanel_endpoint, QString> >::const_iterator i;
    for(i = m_waiting.constBegin(); i != m_waiting.constEnd(); ++i) {
        emit signalMessage(i->first, i->second);
    }
    m_waiting.clear();
    flush();
}

} /* namespace details */
} /* namespace mixpanel */
