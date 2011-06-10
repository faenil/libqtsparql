/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <tracker-sparql.h>

#include "qsparql_tracker_direct_update_result_p.h"
#include "qsparql_tracker_direct.h"
#include "qsparql_tracker_direct_driver_p.h"
#include "qsparql_tracker_direct_result_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlqueryoptions.h>
#include <qsparqlresultrow.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

////////////////////////////////////////////////////////////////////////////

class QTrackerDirectUpdateResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerDirectUpdateResultPrivate(QTrackerDirectUpdateResult* result, QTrackerDirectDriverPrivate *dpp,
                                      const QSparqlQueryOptions& options);

    ~QTrackerDirectUpdateResultPrivate();
    void startUpdate(const QString& query);
    Q_INVOKABLE void terminate();
    void setLastError(const QSparqlError& e);
    bool checkConnection(const char* errorMsg);

private Q_SLOTS:
    void driverClosing();

public:
    enum State {
        Idle, Executing, Finished
    };
    State state;
    QEventLoop *loop;

    QTrackerDirectUpdateResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
    QSparqlQueryOptions options;
};

static void
async_update_callback( GObject *source_object,
                       GAsyncResult *result,
                       gpointer user_data)
{
    Q_UNUSED(source_object);
    QTrackerDirectUpdateResultPrivate *data = static_cast<QTrackerDirectUpdateResultPrivate*>(user_data);
    if (!data->q) {
        // The user has deleted the Result object before this callback was
        // called. Just delete the ResultPrivate here and do nothing.
        delete data;
        return;
    }

    if (data->driverPrivate) {
        GError *error = 0;
        tracker_sparql_connection_update_finish(data->driverPrivate->connection, result, &error);

        if (error) {
            QSparqlError e(QString::fromUtf8(error->message));
            e.setType(errorCodeToType(error->code));
            e.setNumber(error->code);
            data->setLastError(e);
            g_error_free(error);
        }
    }

    // A workaround for http://bugreports.qt.nokia.com/browse/QTBUG-18434

    // We cannot emit the QSparqlResult::finished() signal directly here; so
    // delay it and emit it the next time the main loop spins.
    QMetaObject::invokeMethod(data, "terminate", Qt::QueuedConnection);
}

QTrackerDirectUpdateResultPrivate::QTrackerDirectUpdateResultPrivate(QTrackerDirectUpdateResult* result,
                                                                     QTrackerDirectDriverPrivate *dpp,
                                                                     const QSparqlQueryOptions& options)
  : state(Idle), loop(0),
  q(result), driverPrivate(dpp), options(options)
{
    connect(driverPrivate->driver, SIGNAL(closing()),
            this,                  SLOT(driverClosing()),
            Qt::DirectConnection);
}

QTrackerDirectUpdateResultPrivate::~QTrackerDirectUpdateResultPrivate()
{
}

void QTrackerDirectUpdateResultPrivate::startUpdate(const QString& query)
{
    tracker_sparql_connection_update_async( driverPrivate->connection,
                                            query.toUtf8().constData(),
                                            0,
                                            0,
                                            async_update_callback,
                                            this);
    state = QTrackerDirectUpdateResultPrivate::Executing;
}

void QTrackerDirectUpdateResultPrivate::terminate()
{
    state = Finished;
    if (q->hasError())
        qWarning() << "QTrackerDirectUpdateResult:" << q->lastError() << q->query();
    q->Q_EMIT finished();

    if (loop)
        loop->exit();
}

void QTrackerDirectUpdateResultPrivate::setLastError(const QSparqlError& e)
{
    q->setLastError(e);
}

bool QTrackerDirectUpdateResultPrivate::checkConnection(const char* errorMsg)
{
    if (!driverPrivate || !driverPrivate->connection) {
        setLastError(QSparqlError(QString::fromUtf8(errorMsg),
                                  QSparqlError::ConnectionError));
        return false;
    }
    else {
        return true;
    }
}

void QTrackerDirectUpdateResultPrivate::driverClosing()
{
    driverPrivate = 0;

    QString withQuery;
    if (q)
       withQuery = QString::fromUtf8(" with update query: \"%1\"").arg(q->query());
    qWarning().nospace() << "QSparqlConnection closed before QSparqlResult" << qPrintable(withQuery);
}


////////////////////////////////////////////////////////////////////////////

QTrackerDirectUpdateResult::QTrackerDirectUpdateResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type,
                                           const QSparqlQueryOptions& options)
{
    setQuery(query);
    setStatementType(type);
    d = new QTrackerDirectUpdateResultPrivate(this, p, options);
}

QTrackerDirectUpdateResult::~QTrackerDirectUpdateResult()
{
    if (d->state == QTrackerDirectUpdateResultPrivate::Executing) {
        // We're deleting the Result before the async update has
        // finished. There is a pending async callback that will be called at
        // some point, and that will have our ResultPrivate as user_data. Don't
        // delete the ResultPrivate here, but just mark that we're no longer
        // alive. The callback will then delete the ResultPrivate.
        d->q = 0;
        return;
    }

    delete d;
}

void QTrackerDirectUpdateResult::exec()
{
    if (d->state != QTrackerDirectUpdateResultPrivate::Idle)
        return;

    if (!d->driverPrivate)
        return;

    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
        return;
    }

    d->startUpdate(query());
}

QSparqlBinding QTrackerDirectUpdateResult::binding(int /*field*/) const
{
    return QSparqlBinding();
}

QVariant QTrackerDirectUpdateResult::value(int /*field*/) const
{
    return QVariant();
}

void QTrackerDirectUpdateResult::waitForFinished()
{
    if (isFinished())
        return;

    if (d->driverPrivate) {
        // We first need the connection to be ready before doing anything
        d->driverPrivate->waitForConnectionOpen();

        if (!d->driverPrivate->driver->isOpen()) {
            setLastError(QSparqlError(d->driverPrivate->error,
                                      QSparqlError::ConnectionError));
            d->terminate();
            return;
        }
    }
    else {
        if (d->state == QTrackerDirectUpdateResultPrivate::Idle)
            return;
    }

    QEventLoop loop;
    d->loop = &loop;
    loop.exec();
    d->loop = 0;
}

bool QTrackerDirectUpdateResult::isFinished() const
{
    return (d->state == QTrackerDirectUpdateResultPrivate::Finished);
}

int QTrackerDirectUpdateResult::size() const
{
    return 0;
}

QSparqlResultRow QTrackerDirectUpdateResult::current() const
{
    return QSparqlResultRow();
}

QT_END_NAMESPACE

#include "qsparql_tracker_direct_update_result_p.moc"
