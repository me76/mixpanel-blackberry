/*
 * MessageStore.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: joe
 */

#include "MessageStore.h"

#include <iostream>

#include <QFile>
#include <QTime>

// TODO Find QTSingleton or what have you, ALL of these should be singletons except for Mixpanel;

#define DATABASE_PATH "data/mixpanel_messages.db"
#define DATABASE_DRIVER "QSQLITE"
#define DATABASE_CONNECTION_NAME "mixpanel_messages_connection"

namespace mixpanel {
namespace details {

MessageStore::MessageStore() {}

MessageStore::~MessageStore() {}

bool MessageStore::deleteDb() {
    unconnect();
    QFile db_file(DATABASE_PATH);
    if (! db_file.exists()) {
        return true; // if it never existed in the first place, then it's still deleted
    }
    return db_file.remove();
}

void MessageStore::unconnect() {
    if (m_connection.isOpen()) {
        QSqlDatabase::removeDatabase(DATABASE_PATH);
        m_connection.removeDatabase(DATABASE_DRIVER);
    }
}

bool MessageStore::store(enum mixpanel_endpoint endpoint, const QString &message) {
    QSqlDatabase *db = getConnection();
    if (db == NULL) {
        return false;
    }
    QDateTime now = QDateTime::currentDateTime();
    QSqlQuery query(*db);
    query.prepare(
        "INSERT INTO messages_v0 (_id, endpoint, message, created_at) "
        " VALUES (NULL, :endpoint, :message, :created_at)"
    );
    query.bindValue(":endpoint", endpoint);
    query.bindValue(":message", message);
    query.bindValue(":created_at", now.toMSecsSinceEpoch());
    query.exec();
    const QSqlError err = query.lastError();
    if (err.isValid()) {
        unconnect();
        return false;
    }
    return true;
}

bool MessageStore::retrieve(enum mixpanel_endpoint endpoint, int limit, QList<QString> *results, int *last_id) {
    QSqlDatabase *db = getConnection();
    if (NULL == db) {
        return false;
    }
    QSqlQuery query(*db);
    query.prepare(
        "SELECT _id, message FROM messages_v0 WHERE endpoint = :endpoint "
        " ORDER BY _id ASC LIMIT :limit"
    );
    query.bindValue(":endpoint", endpoint);
    query.bindValue(":limit", limit); // TODO is this going to work?
    query.exec();
    const QSqlError err = query.lastError();
    if (err.isValid()) {
        unconnect();
        return false;
    }
    results->clear();
    while (query.next()) {
        *last_id = query.value(0).toInt();
        results->append(query.value(1).toString());
    }
    return true;
}

bool MessageStore::count(enum mixpanel_endpoint endpoint, int *count) {
    QSqlDatabase *db = getConnection();
    if (NULL == db) {
        return false;
    }
    QSqlQuery query(*db);
    if (! query.prepare("SELECT COUNT(*) FROM messages_v0 WHERE endpoint = :endpoint")) {
        unconnect();
        return false;
    }
    query.bindValue(":endpoint", endpoint);
    query.exec();
    const QSqlError err = query.lastError();
    if (err.isValid()) {
        unconnect();
        return false;
    }
    if (! query.next()) {
        unconnect();
        return false;
    }
    *count = query.value(0).toInt(); // TODO what if this fails (because result is a string or something?)
    return true;
}

bool MessageStore::clearMessagesUptoId(enum mixpanel_endpoint endpoint, int id) {
    QSqlDatabase *db = getConnection();
    if (NULL == db) {
        return false;
    }
    QSqlQuery query(*db);
    if (! query.prepare("DELETE FROM messages_v0 WHERE _id <= :clear_id AND endpoint = :endpoint")) {
        unconnect();
        return false;
    }
    query.bindValue(":endpoint", endpoint);
    query.bindValue(":clear_id", id);
    query.exec();
    const QSqlError err = query.lastError();
    if (err.isValid()) {
        unconnect();
        return false;
    }
    return true;
}

bool MessageStore::clearMessagesUptoTime(std::time_t time_val) {
    QSqlDatabase *db = getConnection();
    if (NULL == db) {
        return false;
    }
    QSqlQuery query(*db);
    if (! query.prepare("DELETE FROM messages_v0 WHERE created_at <= :clear_time")) {
        unconnect();
        return false;
    }
    query.bindValue(":clear_time", time_val);
    query.exec();
    const QSqlError err = query.lastError();
    if (err.isValid()) {
        unconnect();
        return false;
    }
    return true;
}

QSqlDatabase* MessageStore::getConnection() {
    if (m_connection.isOpen()) {
        return &m_connection;
    }
    if (QSqlDatabase::contains(DATABASE_CONNECTION_NAME)){
        std::cerr << "Database is already open: " << m_connection.lastError().text().toUtf8().constData();
        return NULL;
    }
    m_connection = QSqlDatabase::addDatabase(DATABASE_DRIVER, DATABASE_CONNECTION_NAME); // TODO Check docs, can this fail?
    m_connection.setDatabaseName(DATABASE_PATH);
    if (!m_connection.isValid()) { // Invalid driver
        return NULL;
    } else if (!m_connection.open()) {
        return NULL;
    }
    const QString table_check("SELECT name FROM sqlite_master WHERE type='table' AND name='messages_v0'");
    QSqlQuery table_check_query(table_check, m_connection);
    const QSqlError table_check_err = table_check_query.lastError();
    if (table_check_err.isValid()) {
        return NULL;
    } else if (! table_check_query.next()) {
        if (!initializeDatabase() ) {
            return NULL;
        }
    }
    return &m_connection;
}

bool MessageStore::initializeDatabase() {
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
        return false;
    }
    const QString time_index_create(
            "CREATE INDEX IF NOT EXISTS time_idx ON messages_v0 (created_at)"
            );
    QSqlQuery time_index_create_query(time_index_create, m_connection);
    const QSqlError time_index_create_err = time_index_create_query.lastError();
    if (time_index_create_err.isValid()) {
        return false;
    }
    const QString endpoint_index_create(
            "CREATE INDEX IF NOT EXISTS endpoint_idx ON messages_v0 (endpoint)"
            );
    const QSqlQuery endpoint_index_create_query(endpoint_index_create, m_connection);
    const QSqlError endpoint_index_create_err = endpoint_index_create_query.lastError();
    if (endpoint_index_create_err.isValid()) {
        return false;
    }
    // TODO need an _id,endpoint compound index (if SQLite supports such things)
    return true;
}

} /* namespace details */
} /* namespace mixpanel */
