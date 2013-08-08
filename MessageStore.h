/*
 * MessageStore.h
 *
 *  Created on: Aug 3, 2013
 *      Author: joe
 */

#ifndef MESSAGESTORE_H_
#define MESSAGESTORE_H_

#include "mixpanel_endpoint.h"
#include <QList>
#include <QString>
#include <QtSql/QtSql>

#include <ctime>

namespace mixpanel {
namespace details {

class MessageStore {
public:
    MessageStore();
    virtual ~MessageStore();
    bool store(enum mixpanel_endpoint endpoint, const QString &message);
    bool retrieve(enum mixpanel_endpoint endpoint, QList<QString> *results, int *last_id);
    bool count(enum mixpanel_endpoint endpoint, int *count);
    bool clearMessagesUptoId(enum mixpanel_endpoint endpoint, int id);
    bool clearMessagesUptoTime(std::time_t time);
    bool deleteDb();
private:
    MessageStore(const MessageStore&);
    MessageStore& operator=(const MessageStore&);
    QSqlDatabase* getConnection();
    bool initializeDatabase();
    void unconnect();
    QSqlDatabase m_connection;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGESTORE_H_ */
