/*
 * MessageThread.h
 *
 *  Created on: Aug 5, 2013
 *      Author: joe
 */

#ifndef MESSAGETHREAD_H_
#define MESSAGETHREAD_H_

#include "MessageWorker.h"

#include <utility>

#include <QObject>
#include <QThread>
#include <QMutex>

namespace mixpanel {
namespace details {

enum task_type {
    TASK_TYPE_MESSAGE,
    TASK_TYPE_FLUSH,
    TASK_TYPE_DIE
};

class MessageThread: public QThread {
public:
    MessageThread();
    ~MessageThread();

    void message(enum mixpanel_endpoint endpoint, const QString &message);
    void flush(int connect_timeout);
    void stopBlocking();
    void stopNonblocking();
    int getDepth();

protected:
    void run();

private:
    MessageThread(const MessageThread&);
    MessageThread& operator=(const MessageThread&);
    struct task {
        task() {}
        task(enum task_type type, enum mixpanel_endpoint endpoint, const QString &message, int intmessage) {
            this->task_type = type;
            this->endpoint = endpoint;
            this->message = message;
            this->intmessage = intmessage;
        }
        enum task_type task_type;
        enum mixpanel_endpoint endpoint;
        QString message;
        int intmessage;
    };
    const task m_die_task;
    QQueue<struct task> m_queue;
    QMutex m_queue_mutex;
    QWaitCondition m_wait_condition;
    bool m_dying;
    int m_depth;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGETHREAD_H_ */
