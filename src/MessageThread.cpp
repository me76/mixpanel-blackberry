/*
 * MessageThread.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: joe
 */

#include "MessageThread.h"
#include <iostream>
#include <time.h>

#include <QMutexLocker>

namespace mixpanel {
namespace details {

#define FLUSH_INTERVAL_SECONDS 60

MessageThread::MessageThread()
    : QThread(), m_queue(), m_queue_mutex() {
    setObjectName(QString("MixpanelMessageThread_InternalThread"));
    start();
}

MessageThread::~MessageThread() {
    static QString empty_string;
    struct task die_task;
    die_task.task_type = TASK_TYPE_DIE;
    die_task.endpoint = MIXPANEL_ENDPOINT_UNDEFINED;
    die_task.message = empty_string;
    QMutexLocker lock(&m_queue_mutex);
    m_queue.clear(); // These messages are already dead, since our storage and network stuff may be gone.
    m_queue.append(die_task);
    m_wait_condition.wakeOne();
    lock.unlock();
    this->wait();
}

// Called from client threads, must be thread-safe
void MessageThread::message(enum mixpanel_endpoint endpoint, const QString &message) {
    struct task next_task;
    next_task.task_type = TASK_TYPE_MESSAGE;
    next_task.endpoint = endpoint;
    next_task.message = QString(message.data()); // Force deep copy
    QMutexLocker lock(&m_queue_mutex);
    m_queue.append(next_task);
    m_wait_condition.wakeOne();
}

// Called from client threads, must be thread-safe
void MessageThread::flush() {
    static QString empty_string;
    struct task next_task;
    next_task.task_type = TASK_TYPE_FLUSH;
    next_task.endpoint = MIXPANEL_ENDPOINT_UNDEFINED;
    next_task.message = empty_string;
    QMutexLocker lock(&m_queue_mutex);
    m_queue.enqueue(next_task);
    m_wait_condition.wakeOne();
}

void MessageThread::run() {
    MessageWorker worker;
    struct task next_task;
    time_t now = time(NULL);
    time_t last_flush = now;
    for (;;) {

        QMutexLocker lock(&m_queue_mutex);
        for (;;) {
            if (! m_queue.isEmpty()) break;
            if (now - last_flush > FLUSH_INTERVAL_SECONDS) break;
            m_wait_condition.wait(&m_queue_mutex, FLUSH_INTERVAL_SECONDS * 1000);
            now = time(NULL);
        }
        if (m_queue.isEmpty()) {
            next_task.task_type = TASK_TYPE_FLUSH;
            next_task.endpoint = MIXPANEL_ENDPOINT_UNDEFINED;
        } else {
            next_task = m_queue.dequeue();
        }
        lock.unlock();

        switch (next_task.task_type) {
        case TASK_TYPE_MESSAGE:
            worker.message(next_task.endpoint, next_task.message);
            break;
        case TASK_TYPE_FLUSH:
            worker.flush();
            last_flush = time(NULL);
            break;
        case TASK_TYPE_DIE:
            return;
        }
        now = time(NULL);

    }
}

} /* namespace details */
} /* namespace mixpanel */
