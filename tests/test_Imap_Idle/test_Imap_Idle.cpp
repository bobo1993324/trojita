/* Copyright (C) 2006 - 2011 Jan Kundrát <jkt@gentoo.org>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtTest>
#include "test_Imap_Idle.h"
#include "../headless_test.h"
#include "Streams/FakeSocket.h"
#include "Imap/Tasks/IdleLauncher.h"
#include "Imap/Model/ItemRoles.h"
#include "Imap/Tasks/KeepMailboxOpenTask.h"
#include "test_LibMailboxSync/FakeCapabilitiesInjector.h"

/** @short Test a NO reply to IDLE command */
void ImapModelIdleTest::testIdleNo()
{
    model->setProperty("trojita-imap-idle-delayedEnter", QVariant(30));
    FakeCapabilitiesInjector injector(model);
    injector.injectCapability(QLatin1String("IDLE"));
    existsA = 3;
    uidValidityA = 6;
    uidMapA << 1 << 7 << 9;
    uidNextA = 16;
    helperSyncAWithMessagesEmptyState();
    QVERIFY(SOCK->writtenStuff().isEmpty());
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(t.last("NO you can't idle now\r\n"));
    QTest::qWait(40);
    QVERIFY(SOCK->writtenStuff().isEmpty());
    helperSyncBNoMessages();
    QVERIFY(errorSpy->isEmpty());
}

/** @short Test what happens when IDLE terminates by an OK, but without our "DONE" input */
void ImapModelIdleTest::testIdleImmediateReturn()
{
    model->setProperty("trojita-imap-idle-delayedEnter", QVariant(30));
    FakeCapabilitiesInjector injector(model);
    injector.injectCapability(QLatin1String("IDLE"));
    existsA = 3;
    uidValidityA = 6;
    uidMapA << 1 << 7 << 9;
    uidNextA = 16;
    helperSyncAWithMessagesEmptyState();
    QVERIFY(SOCK->writtenStuff().isEmpty());
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(QByteArray("+ blah\r\n") + t.last("OK done\r\n"));
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
}

/** @short Test automatic IDLE renewal */
void ImapModelIdleTest::testIdleRenewal()
{
    model->setProperty("trojita-imap-idle-delayedEnter", QVariant(30));
    model->setProperty("trojita-imap-idle-renewal", QVariant(10));
    FakeCapabilitiesInjector injector(model);
    injector.injectCapability(QLatin1String("IDLE"));
    existsA = 3;
    uidValidityA = 6;
    uidMapA << 1 << 7 << 9;
    uidNextA = 16;
    helperSyncAWithMessagesEmptyState();
    QVERIFY(SOCK->writtenStuff().isEmpty());
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(QByteArray("+ blah\r\n"));
    QTest::qWait(10);
    QCOMPARE( SOCK->writtenStuff(), QByteArray("DONE\r\n") );
    SOCK->fakeReading(t.last("OK done\r\n"));
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
}

/** @short Test that IDLE gets immediately interrupted by any Task */
void ImapModelIdleTest::testIdleBreakTask()
{
    model->setProperty("trojita-imap-idle-delayedEnter", QVariant(30));
    // Intentionally leave trojita-imap-idle-renewal at its rather high default value
    FakeCapabilitiesInjector injector(model);
    injector.injectCapability(QLatin1String("IDLE"));
    existsA = 3;
    uidValidityA = 6;
    uidMapA << 1 << 7 << 9;
    uidNextA = 16;
    helperSyncAWithMessagesEmptyState();
    QVERIFY(SOCK->writtenStuff().isEmpty());
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(QByteArray("+ blah\r\n"));
    QCOMPARE( msgListA.child(0,0).data(Imap::Mailbox::RoleMessageFrom).toString(), QString() );
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE( SOCK->writtenStuff(), QByteArray("DONE\r\n") + t.mk("UID FETCH 1,7,9 (ENVELOPE BODYSTRUCTURE RFC822.SIZE)\r\n") );
    SOCK->fakeReading(t.last("OK done\r\n"));
    QTest::qWait(40);
    QVERIFY(SOCK->writtenStuff().isEmpty());
}

/** @short Test automatic IDLE renewal when server gets really slow to respond */
void ImapModelIdleTest::testIdleSlowResponses()
{
    model->setProperty("trojita-imap-idle-delayedEnter", QVariant(30));
    model->setProperty("trojita-imap-idle-renewal", QVariant(10));
    FakeCapabilitiesInjector injector(model);
    injector.injectCapability(QLatin1String("IDLE"));
    existsA = 3;
    uidValidityA = 6;
    uidMapA << 1 << 7 << 9;
    uidNextA = 16;
    helperSyncAWithMessagesEmptyState();
    QVERIFY(SOCK->writtenStuff().isEmpty());

    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    // Check what happens if it takes the server a lot of time to issue the initial continuation
    QTest::qWait(70);
    SOCK->fakeReading(QByteArray("+ blah\r\n"));
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE( SOCK->writtenStuff(), QByteArray("DONE\r\n") );
    SOCK->fakeReading(t.last("OK done\r\n"));

    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(QByteArray("+ blah\r\n"));
    // The client is fast enough...
    QTest::qWait(40);
    QCOMPARE( SOCK->writtenStuff(), QByteArray("DONE\r\n") );
    // ...but the server is taking its time
    QTest::qWait(70);
    QVERIFY(SOCK->writtenStuff().isEmpty());
    SOCK->fakeReading(t.last("OK done\r\n"));
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QVERIFY(SOCK->writtenStuff().isEmpty());
    //QCOMPARE( SOCK->writtenStuff(), t.mk("DONE\r\n") );
}

/** @short Test that the automatic IDLE renewal gets disabled when IDLE finishes */
void ImapModelIdleTest::testIdleNoPerpetuateRenewal()
{
    // we shouldn't enter IDLE automatically
    model->setProperty("trojita-imap-idle-delayedEnter", QVariant(1000 * 1000 ));
    model->setProperty("trojita-imap-idle-renewal", QVariant(10));
    FakeCapabilitiesInjector injector(model);
    injector.injectCapability(QLatin1String("IDLE"));
    existsA = 3;
    uidValidityA = 6;
    uidMapA << 1 << 7 << 9;
    uidNextA = 16;
    helperSyncAWithMessagesEmptyState();
    QVERIFY(SOCK->writtenStuff().isEmpty());

    // Force manual trigger of the IDLE
    model->findTaskResponsibleFor(idxA)->idleLauncher->slotEnterIdleNow();
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(t.last("NO you can't idle now\r\n"));
    // ...make sure it won't try to "break long IDLE"
    QTest::qWait(30);
    // switch away
    helperSyncBNoMessages();
    QVERIFY(errorSpy->isEmpty());

    // Now go back to mailbox A
    model->switchToMailbox(idxA);
    helperSyncAWithMessagesNoArrivals();

    // Force manual trigger of the IDLE
    model->findTaskResponsibleFor(idxA)->idleLauncher->slotEnterIdleNow();
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE( SOCK->writtenStuff(), t.mk("IDLE\r\n") );
    SOCK->fakeReading(QByteArray("+ blah\r\n"));
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();

    // so we're in regular IDLE and want to break it
    QCOMPARE( msgListA.child(0,0).data(Imap::Mailbox::RoleMessageFrom).toString(), QString() );
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE( SOCK->writtenStuff(), QByteArray("DONE\r\n") + t.mk("UID FETCH 1,7,9 (ENVELOPE BODYSTRUCTURE RFC822.SIZE)\r\n") );
    SOCK->fakeReading(t.last("OK done\r\n"));
    // Make sure we won't try to "renew" it automatically...
    QTest::qWait(30);
    QVERIFY(SOCK->writtenStuff().isEmpty());
}


TROJITA_HEADLESS_TEST( ImapModelIdleTest )
