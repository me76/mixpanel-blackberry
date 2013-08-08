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

class MessageThread: private QThread {
public:
    MessageThread();
    ~MessageThread();
protected:
    void run();
public:
    void message(enum mixpanel_endpoint endpoint, const QString &message);
    void flush();
private:
    MessageThread(const MessageThread&);
    MessageThread& operator=(const MessageThread&);
    struct task {
        enum task_type task_type;
        enum mixpanel_endpoint endpoint;
        QString message;
    };
    QQueue<struct task> m_queue;
    QMutex m_queue_mutex;
    QWaitCondition m_wait_condition;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGETHREAD_H_ */
