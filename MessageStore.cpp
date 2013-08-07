/*
 * MessageStore.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: joe
 */

#include "MessageStore.h"

#include <QFile>
#include <QTime>

#define DATABASE_PATH "data/mixpanel_messages.db"
#define DATABASE_DRIVER "QSQLITE"
#define DATABASE_CONNECTION_NAME "mixpanel_messages_connection"

namespace mixpanel {

namespace details {

MessageStore::MessageStore()
	: READ_QUERY("SELECT _id, message FROM messages_v0 WHERE endpoint = :endpoint"),
	  COUNT_QUERY("SELECT COUNT(*) FROM messages_v0 WHERE endpoint = :endpoint"),
	  WRITE_QUERY(
				"INSERT INTO messages_v0 (_id, endpoint, message, created_at)"
				" VALUES (NULL, :endpoint, :message, :created_at)"
				),
	  CLEAR_BY_TIME("DELETE FROM messages_v0 WHERE created_at <= :clear_time"),
	  CLEAR_BY_ENDPOINT_ID("DELETE FROM messages_v0 WHERE _id <= :clear_id AND endpoint = :endpoint") {
	failed = false;
}

MessageStore::~MessageStore() {
	unconnect();
}

void MessageStore::deleteDb() {
	unconnect();
	QFile db_file(DATABASE_PATH);
	if (db_file.exists()) {
		bool ok = db_file.remove();
		if (! ok) {
			qWarning() << "Couldn't remove the database!\n";
		}
	}
}

void MessageStore::unconnect() {
	if (m_connection.isOpen()) {
        QSqlDatabase::removeDatabase(DATABASE_PATH);
        m_connection.removeDatabase(DATABASE_DRIVER); // TODO ??
	}
}

bool MessageStore::store(enum mixpanel_endpoint endpoint, const QString &message) {
	QSqlDatabase *db = getConnection();
	if (db == NULL) {
		return false; // We're not tracking
	}
	QDateTime now = QDateTime::currentDateTime();
	QSqlQuery query(*db);
	query.prepare(WRITE_QUERY);
	query.bindValue(":endpoint", endpoint);
	query.bindValue(":message", message);
	query.bindValue(":created_at", now.toMSecsSinceEpoch());
	query.exec();
	const QSqlError err = query.lastError();
	if (err.isValid()) {
		unconnect();
		failed = true;
		return false;
	}
	return true;
}

bool MessageStore::retrieve(enum mixpanel_endpoint endpoint, QList<QString> &results, int &last_id) {
	QSqlDatabase *db = getConnection();
	if (NULL == db) {
		return false;
	}
	QSqlQuery query(*db);
	if (! query.prepare(READ_QUERY)) {
		unconnect();
		failed = true;
		return false;
	}
	query.bindValue(":endpoint", endpoint);
	query.exec();
	const QSqlError err = query.lastError();
	if (err.isValid()) {
		unconnect();
		failed = true;
		return false;
	}
	results.clear();
	while (query.next()) {
		last_id = query.value(0).toInt();
		results.append(query.value(1).toString());
	}
	return true;
}

bool MessageStore::count(enum mixpanel_endpoint endpoint, int &count) {
	QSqlDatabase *db = getConnection();
	if (NULL == db) {
		return false;
	}
	QSqlQuery query(*db);
	if (! query.prepare(COUNT_QUERY)) {
		unconnect();
		failed = true;
		return false;
	}
	query.bindValue(":endpoint", endpoint);
	query.exec();
	const QSqlError err = query.lastError();
	if (err.isValid()) {
		unconnect();
		failed = true;
		return false;
	}
	if (! query.next()) {
		unconnect();
		failed = true;
		return false;
	}
	count = query.value(0).toInt();
	return true;
}

bool MessageStore::clearMessagesWithEndpointFromId(enum mixpanel_endpoint endpoint, int id) {
	QSqlDatabase *db = getConnection();
	if (NULL == db) {
		return false;
	}
	QSqlQuery query(*db);
	if (! query.prepare(CLEAR_BY_ENDPOINT_ID)) {
		unconnect();
		failed = true;
		return false;
	}
	query.bindValue(":endpoint", endpoint);
	query.bindValue(":clear_id", id);
	query.exec();
	const QSqlError err = query.lastError();
	if (err.isValid()) {
		unconnect();
		failed = true;
		return false;
	}
	return true;
}

bool MessageStore::clearMessagesFromTime(int time) {
	QSqlDatabase *db = getConnection();
	if (NULL == db) {
		return false;
	}
	QSqlQuery query(*db);
	if (! query.prepare(CLEAR_BY_TIME)) {
		unconnect();
		failed = true;
		return false;
	}
	query.bindValue(":clear_time", time);
	query.exec();
	const QSqlError err = query.lastError();
	if (err.isValid()) {
		unconnect();
		failed = true;
		return false;
	}
	return true;
}

QSqlDatabase* MessageStore::getConnection() {
    if (m_connection.isOpen()) {
        return &m_connection;
    } else if (failed) {
    	return NULL;
    }

    if (QSqlDatabase::contains(DATABASE_CONNECTION_NAME)){
        // if the database connection already exists, then reuse it
        // Calling this also calls "open"
        m_connection = QSqlDatabase::database(DATABASE_CONNECTION_NAME);

        if (!m_connection.isOpen()){
            qWarning() << "Could not open connection to database: " << m_connection.lastError().text();
        }
    } else {
        // Otherwise, the connection doesn't exist, so create it and open it
        m_connection = QSqlDatabase::addDatabase(DATABASE_DRIVER, DATABASE_CONNECTION_NAME);
        m_connection.setDatabaseName(DATABASE_PATH);

        if (!m_connection.isValid()) {
            qDebug() << "Could not set data base name probably due to invalid driver.";
            failed = true;
            return NULL;
        } else if (!m_connection.open()) {
            qWarning() << "Could not open connection to database: " << m_connection.lastError().text();
            failed = true;
            return NULL;
        }

        const QString table_check("SELECT name FROM sqlite_master WHERE type='table' AND name='messages_v0'");
        QSqlQuery table_check_query(table_check, m_connection);
        const QSqlError table_check_err = table_check_query.lastError();
        qDebug() << "Checking for message table";
        if (table_check_err.isValid()) {
            qWarning() << "Error executing table check SQL: " << table_check_err.text();
            failed = true;
            return NULL;
        } else if (table_check_query.next()) {
            QString name = table_check_query.value(0).toString();
            qDebug() << "Found name: " << name << "\n";
        } else {
        	qWarning() << "Table messages_v0 doesn't exist - creating it.\n";
        	initializeDatabase();
        	if (failed) {
        		return NULL;
        	}
        }
    }
    return &m_connection;
}

void MessageStore::initializeDatabase() {
	const QString table_create(
			"CREATE TABLE messages_v0 ("
			"   _id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"   endpoint INTEGER NOT NULL,"
			"   message STRING NOT NULL,"
			"   created_at INTEGER NOT NULL"
			")"
			);
	QSqlQuery table_create_query(table_create, m_connection);
	const QSqlError table_create_err = table_create_query.lastError();
	if (table_create_err.isValid()) {
        qWarning() << "Error creating table: " << table_create_err.text();
		failed = true;
		return;
	}
	const QString time_index_create(
			"CREATE INDEX IF NOT EXISTS time_idx ON messages_v0 (created_at)"
			);
	QSqlQuery time_index_create_query(time_index_create, m_connection);
	const QSqlError time_index_create_err = time_index_create_query.lastError();
	if (time_index_create_err.isValid()) {
		qWarning() << "Couldn't create time index on mixpanel table: " << time_index_create_err.text();
		failed = true;
		return;
	}
	const QString endpoint_index_create(
			"CREATE INDEX IF NOT EXISTS endpoint_idx ON messages_v0 (endpoint)"
			);
	const QSqlQuery endpoint_index_create_query(endpoint_index_create, m_connection);
	const QSqlError endpoint_index_create_err = endpoint_index_create_query.lastError();
	if (endpoint_index_create_err.isValid()) {
		qWarning() << "Couldn't create endpoint index on mixpanel table: " << time_index_create_err.text();
		failed = true;
		return;
	}
}

} /* namespace details */

} /* namespace mixpanel */
