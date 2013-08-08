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
    : QThread(), m_worker(), m_queue(), m_queue_mutex() {
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
    m_queue.append(die_task);
    m_wait_condition.wakeOne();
    lock.unlock();
    this->wait();
}

void MessageThread::message(enum mixpanel_endpoint endpoint, const QString &message) {
    struct task next_task;
    next_task.task_type = TASK_TYPE_MESSAGE;
    next_task.endpoint = endpoint;
    next_task.message = QString(message.data()); // Force deep copy
    QMutexLocker lock(&m_queue_mutex);
    m_queue.append(next_task);
    m_wait_condition.wakeOne();
}

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
    struct task next_task;
    while (TASK_TYPE_DIE != next_task.task_type) {
        QMutexLocker lock(&m_queue_mutex);
        while (m_queue.isEmpty()) {
            m_wait_condition.wait(&m_queue_mutex); // TODO timeout?
        }
        next_task = m_queue.dequeue();
        lock.unlock();
        switch (next_task.task_type) {
        case TASK_TYPE_MESSAGE:
            m_worker.message(next_task.endpoint, next_task.message);
            break;
        case TASK_TYPE_FLUSH:
            m_worker.flush();
            break;
        case TASK_TYPE_DIE:
            m_worker.flush();
            return;
        }
    }
}

} /* namespace details */
} /* namespace mixpanel */
