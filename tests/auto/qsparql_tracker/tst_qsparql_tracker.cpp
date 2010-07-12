/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class tst_QSparqlTracker : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlTracker();
    virtual ~tst_QSparqlTracker();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void get_contacts();
    void remove_contact();
    void add_contact();
};

tst_QSparqlTracker::tst_QSparqlTracker()
{
}

tst_QSparqlTracker::~tst_QSparqlTracker()
{
}

void tst_QSparqlTracker::initTestCase()
{
}

void tst_QSparqlTracker::cleanupTestCase()
{
}

void tst_QSparqlTracker::init()
{
}

void tst_QSparqlTracker::cleanup()
{
}

void tst_QSparqlTracker::get_contacts()
{
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    QMap<QString, QString> contactNames; // ordered
    while (r->next()) {
        QCOMPARE(r->resultRow().count(), 2);
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));
}

void tst_QSparqlTracker::add_contact()
{
}

void tst_QSparqlTracker::remove_contact()
{
}


QTEST_MAIN( tst_QSparqlTracker )
#include "tst_qsparql_tracker.moc"
