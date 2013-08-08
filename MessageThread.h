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

class MessageThread: private QObject {
    Q_OBJECT
public:
    MessageThread();
    ~MessageThread();

public:
    void message(enum mixpanel_endpoint endpoint, const QString &message);
    void flush();
signals:
    void signalMessage(enum mixpanel_endpoint endpoint, const QString &message);
    void signalFlush();
private slots:
    void threadStarted();
    void pushWaiting();
private:
    MessageThread(const MessageThread&);
    MessageThread& operator=(const MessageThread&);
    QThread m_thread;
    MessageWorker m_worker;
    QList< std::pair<enum mixpanel_endpoint, QString> > m_waiting;
    bool m_running;
    QMutex m_running_mutex;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGETHREAD_H_ */
