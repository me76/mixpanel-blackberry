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

namespace mixpanel {
namespace details {

class MessageStore {
public:
	MessageStore();
	virtual ~MessageStore();
    bool store(enum mixpanel_endpoint endpoint, const QString &message);
    bool retrieve(enum mixpanel_endpoint endpoint, QList<QString> &results, int &last_id);
    bool count(enum mixpanel_endpoint endpoint, int &count);
    bool clearMessagesWithEndpointFromId(enum mixpanel_endpoint endpoint, int id);
    bool clearMessagesFromTime(int time);
    void deleteDb();

private:
    static const QString WRITE_QUERY;
    static const QString READ_QUERY;
    static const QString COUNT_QUERY;
    static const QString CLEAR_BY_TIME;
    static const QString CLEAR_BY_ENDPOINT_ID;
    QSqlDatabase* getConnection();
    void initializeDatabase();
    void unconnect();
    QSqlDatabase m_connection;
    bool failed;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGESTORE_H_ */
