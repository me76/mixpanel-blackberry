/*
 * MessageThread.h
 *
 *  Created on: Aug 5, 2013
 *      Author: joe
 */

#ifndef MESSAGETHREAD_H_
#define MESSAGETHREAD_H_

#include "MessageWorker.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace mixpanel {
namespace details {

class MessageThread: private QObject {
	Q_OBJECT
public:
	MessageThread();
	virtual ~MessageThread();
public:
	void message(enum mixpanel_endpoint endpoint, QString message);
	void flush();
signals:
	void signalMessage(enum mixpanel_endpoint endpoint, QString message);
	void signalFlush();
private slots:
	void threadStarted();
	void threadFinished();
private:
	QThread *m_thread;
	MessageWorker *m_worker;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGETHREAD_H_ */
