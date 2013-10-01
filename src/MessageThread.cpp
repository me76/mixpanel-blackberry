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
    : QThread(),
      m_die_task(TASK_TYPE_DIE, MIXPANEL_ENDPOINT_UNDEFINED, QString(""), -1),
      m_queue(), m_queue_mutex(), m_dead(false) {
    setObjectName(QString("MixpanelMessageThread_InternalThread"));
    start();
}

MessageThread::~MessageThread() {
    QMutexLocker lock(&m_queue_mutex);
    if (!m_dead) {
        // At this point, our underlying storage and network libraries may have become unstable.
        // We drop everything else in the queue and command the thread to die.
        m_queue.clear();
        m_queue.append(m_die_task);
        m_dead = true;
        m_wait_condition.wakeOne();
        lock.unlock();
        this->wait();
    }
}

// Called from client threads, must be thread-safe
void MessageThread::message(enum mixpanel_endpoint endpoint, const QString &message) {
    QString message_copy = QString(message.data()); // Force deep copy
    struct task next_task(TASK_TYPE_MESSAGE, endpoint, message_copy, -1);
    QMutexLocker lock(&m_queue_mutex);
    m_queue.append(next_task);
    m_wait_condition.wakeOne();
}

// Called from client threads, must be thread-safe
void MessageThread::flush(int connect_timeout) {
	struct task flush_task(TASK_TYPE_FLUSH, MIXPANEL_ENDPOINT_UNDEFINED, QString(""), connect_timeout);
    QMutexLocker lock(&m_queue_mutex);
    m_queue.enqueue(flush_task);
    m_wait_condition.wakeOne();
}

void MessageThread::stop() {
    QMutexLocker lock(&m_queue_mutex);
    if (!m_dead) {
        m_queue.append(m_die_task);
        m_dead = true;
        m_wait_condition.wakeOne();
        lock.unlock();
        this->wait();
    }
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
            worker.flush(next_task.intmessage);
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
