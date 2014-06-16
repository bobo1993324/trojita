/* Copyright (C) 2006 - 2014 Jan Kundrát <jkt@flaska.net>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UiUtils/Formatting.h"
#include <cmath>
#include <QSslError>
#include <QSslKey>
#include <QStringList>
#include <QTextDocument>
#include <QFontInfo>
#include <QFileInfo>
#include <QFile>
#include "Common/Paths.h"
#include "UiUtils/Color.h"

namespace UiUtils {

Formatting::Formatting(QObject *parent): QObject(parent)
{
}

QString Formatting::prettySize(uint bytes, const BytesSuffix compactUnitFormat)
{
    if (bytes == 0) {
        return tr("0");
    }
    int order = std::log(static_cast<double>(bytes)) / std::log(1024.0);
    double number = bytes / std::pow(1024.0, order);

    QString suffix;
    if (order <= 0) {
        if (compactUnitFormat == BytesSuffix::COMPACT_FORM)
            return QString::number(bytes);
        else
            return tr("%1 bytes").arg(QString::number(bytes));
    } else if (order == 1) {
        suffix = tr("kB");
    } else if (order == 2) {
        suffix = tr("MB");
    } else if (order == 3) {
        suffix = tr("GB");
    } else {
        // make sure not to show wrong size for those that have > 1024 TB e-mail messages
        suffix = tr("TB"); // shame on you for such mails
    }
    return tr("%1 %2").arg(QString::number(number, 'f', number < 100 ? 1 : 0), suffix);
}

/** @short Format a QDateTime for compact display in one column of the view */
QString Formatting::prettyDate(const QDateTime &dateTime)
{
    // The time is not always synced properly, so better accept even slightly too new messages as "from today"
    QDateTime now = QDateTime::currentDateTime().addSecs(15*60);
    if (dateTime >= now) {
        // Messages from future shall always be shown using full format to prevent nasty surprises.
        return dateTime.toString(Qt::DefaultLocaleShortDate);
    } else if (dateTime.date() == now.date() || dateTime > now.addSecs(-6 * 3600)) {
        // It's a "today's message", i.e. something which is either literally from today or at least something not older than
        // six hours (an arbitraty magic number).
        // Originally, the cut-off time interval was set to 24 hours, but it led to weird things in the GUI like showing mails
        // from yesterday's 18:33 just as "18:33" even though the local time was "18:20" already. In a perfect world, we would
        // also periodically emit dataChanged() in order to force a wrap once the view has been open for too long, but that will
        // have to wait a bit.
        // The time is displayed without seconds to conserve space as well.
        //: please do not translate the format specifier (you can change their order
        //: or the separator to follow the local conventions)
        return dateTime.time().toString(tr("hh:mm"));
    } else if (dateTime > now.addDays(-7)) {
        // Messages from the last seven days can be formatted just with the weekday name
        //: please do not translate the format specifier (you can change their order
        //: or the separator to follow the local conventions)
        return dateTime.toString(tr("ddd hh:mm"));
    } else if (dateTime > now.addYears(-1)) {
        // Messages newer than one year don't have to show year
        //: please do not translate the format specifier (you can change their order
        //: or the separator to follow the local conventions)
        return dateTime.toString(tr("d MMM hh:mm"));
    } else {
        // Old messagees shall have a full date
        return dateTime.toString(Qt::DefaultLocaleShortDate);
    }
}


/** @short Produce a properly formatted HTML string which won't overflow the right edge of the display */
QString Formatting::htmlHexifyByteArray(const QByteArray &rawInput)
{
    QByteArray inHex = rawInput.toHex();
    QByteArray res;
    const int stepping = 4;
    for (int i = 0; i < inHex.length(); i += stepping) {
        // The individual blocks are formatted separately to allow line breaks to happen
        res.append("<code style=\"font-family: monospace;\">");
        res.append(inHex.mid(i, stepping));
        if (i + stepping < inHex.size()) {
            res.append(":");
        }
        // Produce the smallest possible space. "display: none" won't notice the space at all, leading to overly long lines
        res.append("</code><span style=\"font-size: 1px\"> </span>");
    }
    return QString::fromUtf8(res);
}

QString Formatting::sslChainToHtml(const QList<QSslCertificate> &sslChain)
{
    QStringList certificateStrings;
    Q_FOREACH(const QSslCertificate &cert, sslChain) {
        certificateStrings << tr("<li><b>CN</b>: %1,<br/>\n<b>Organization</b>: %2,<br/>\n"
                                 "<b>Serial</b>: %3,<br/>\n"
                                 "<b>SHA1</b>: %4,<br/>\n<b>MD5</b>: %5</li>").arg(
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                                  cert.subjectInfo(QSslCertificate::CommonName).join(tr(", ")).toHtmlEscaped(),
                                  cert.subjectInfo(QSslCertificate::Organization).join(tr(", ")).toHtmlEscaped(),
#else
                                  Qt::escape(cert.subjectInfo(QSslCertificate::CommonName)),
                                  Qt::escape(cert.subjectInfo(QSslCertificate::Organization)),
#endif
                                  QString::fromUtf8(cert.serialNumber()),
                                  htmlHexifyByteArray(cert.digest(QCryptographicHash::Sha1)),
                                  htmlHexifyByteArray(cert.digest(QCryptographicHash::Md5)));
    }
    return sslChain.isEmpty() ?
                tr("<p>The remote side doesn't have a certificate.</p>\n") :
                tr("<p>This is the certificate chain of the connection:</p>\n<ul>%1</ul>\n").arg(certificateStrings.join(tr("\n")));
}

QString Formatting::sslErrorsToHtml(const QList<QSslError> &sslErrors)
{
    QStringList sslErrorStrings;
    Q_FOREACH(const QSslError &e, sslErrors) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        sslErrorStrings << tr("<li>%1</li>").arg(e.errorString().toHtmlEscaped());
#else
        sslErrorStrings << tr("<li>%1</li>").arg(Qt::escape(e.errorString()));
#endif
    }
    return sslErrors.isEmpty() ?
                tr("<p>According to your system's policy, this connection is secure.</p>\n") :
                tr("<p>The connection triggered the following SSL errors:</p>\n<ul>%1</ul>\n").arg(sslErrorStrings.join(tr("\n")));
}

void Formatting::formatSslState(const QList<QSslCertificate> &sslChain, const QByteArray &oldPubKey,
                                      const QList<QSslError> &sslErrors, QString *title, QString *message, IconType *icon)
{
    bool pubKeyHasChanged = !oldPubKey.isEmpty() && (sslChain.isEmpty() || sslChain[0].publicKey().toPem() != oldPubKey);

    if (pubKeyHasChanged) {
        if (sslErrors.isEmpty()) {
            *icon = IconType::Warning;
            *title = tr("Different SSL certificate");
            *message = tr("<p>The public key of the SSL certificate has changed. "
                          "This should only happen when there was a security incident on the remote server. "
                          "Your system configuration is set to accept such certificates anyway.</p>\n%1\n"
                          "<p>Would you like to connect and remember the new certificate?</p>")
                    .arg(sslChainToHtml(sslChain));
        } else {
            // changed certificate which is not trusted per systemwide policy
            *title = tr("SSL certificate looks fishy");
            *message = tr("<p>The public key of the SSL certificate of the IMAP server has changed since the last time "
                          "and your system doesn't believe that the new certificate is genuine.</p>\n%1\n%2\n"
                          "<p>Would you like to connect anyway and remember the new certificate?</p>").
                    arg(sslChainToHtml(sslChain), sslErrorsToHtml(sslErrors));
            *icon = IconType::Critical;
        }
    } else {
        if (sslErrors.isEmpty()) {
            // this is the first time and the certificate looks valid -> accept
            *title = tr("Accept SSL connection?");
            *message = tr("<p>This is the first time you're connecting to this IMAP server; the certificate is trusted "
                          "by this system.</p>\n%1\n%2\n"
                          "<p>Would you like to connect and remember this certificate's public key for the next time?</p>")
                    .arg(sslChainToHtml(sslChain), sslErrorsToHtml(sslErrors));
            *icon = IconType::Information;
        } else {
            *title = tr("Accept SSL connection?");
            *message = tr("<p>This is the first time you're connecting to this IMAP server and the server certificate failed "
                          "validation test.</p>\n%1\n\n%2\n"
                          "<p>Would you like to connect and remember this certificate's public key for the next time?</p>")
                    .arg(sslChainToHtml(sslChain), sslErrorsToHtml(sslErrors));
            *icon = IconType::Question;
        }
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
QObject *Formatting::factory(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);

    // the reinterpret_cast is used to avoid haivng to depend on QtQuick when doing non-QML builds
    Formatting *f = new Formatting(reinterpret_cast<QObject*>(engine));
    return f;
}
#endif

QString Formatting::markupPlainText(QString plainText, Composer::Util::FlowedFormat flowedFormat, QFont font, QPalette palette)
{
    static const QString defaultStyle = QString::fromUtf8(
        "pre{word-wrap: break-word; white-space: pre-wrap;}"
        // The following line, sadly, produces a warning "QFont::setPixelSize: Pixel size <= 0 (0)".
        // However, if it is not in place or if the font size is set higher, even to 0.1px, WebKit reserves space for the
        // quotation characters and therefore a weird white area appears. Even width: 0px doesn't help, so it looks like
        // we will have to live with this warning for the time being.
        ".quotemarks{color:transparent;font-size:0px;}"

        // Cannot really use the :dir(rtl) selector for putting the quote indicator to the "correct" side.
        // It's CSS4 and it isn't supported yet.
        "blockquote{font-size:90%; margin: 4pt 0 4pt 0; padding: 0 0 0 1em; border-left: 2px solid %1; unicode-bidi: -webkit-plaintext}"

        // Stop the font size from getting smaller after reaching two levels of quotes
        // (ie. starting on the third level, don't make the size any smaller than what it already is)
        "blockquote blockquote blockquote {font-size: 100%}"
        ".signature{opacity: 0.6;}"

        // Dynamic quote collapsing via pure CSS, yay
        "input {display: none}"
        "input ~ span.full {display: block}"
        "input ~ span.short {display: none}"
        "input:checked ~ span.full {display: none}"
        "input:checked ~ span.short {display: block}"
        "label {border: 1px solid %2; border-radius: 5px; padding: 0px 4px 0px 4px; white-space: nowrap}"
        // BLACK UP-POINTING SMALL TRIANGLE (U+25B4)
        // BLACK DOWN-POINTING SMALL TRIANGLE (U+25BE)
        "span.full > blockquote > label:before {content: \"\u25b4\"}"
        "span.short > blockquote > label:after {content: \" \u25be\"}"
        "span.shortquote > blockquote > label {display: none}"
    );

    QFontInfo monospaceInfo(font);
    QString fontSpecification(QLatin1String("pre{"));
    if (monospaceInfo.italic())
        fontSpecification += QLatin1String("font-style: italic; ");
    if (monospaceInfo.bold())
        fontSpecification += QLatin1String("font-weight: bold; ");
    fontSpecification += QString::fromUtf8("font-size: %1px; font-family: \"%2\", monospace }").arg(
                QString::number(monospaceInfo.pixelSize()), monospaceInfo.family());

    QString textColors = QString::fromUtf8("body { background-color: %1; color: %2 }"
                                           "a:link { color: %3 } a:visited { color: %4 } a:hover { color: %3 }").arg(
                palette.base().color().name(), palette.text().color().name(),
                palette.link().color().name(), palette.linkVisited().color().name());
    // looks like there's no special color for hovered links in Qt

    // build stylesheet and html header
    QColor tintForQuoteIndicator = palette.base().color();
    tintForQuoteIndicator.setAlpha(0x66);
    static QString stylesheet = defaultStyle.arg(palette.link().color().name(),
                                                 UiUtils::tintColor(palette.text().color(), tintForQuoteIndicator).name());
    static QFile file(Common::writablePath(Common::LOCATION_DATA) + QLatin1String("message.css"));
    static QDateTime lastVersion;
    QDateTime lastTouched(file.exists() ? QFileInfo(file).lastModified() : QDateTime());
    if (lastVersion < lastTouched) {
        stylesheet = defaultStyle;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString userSheet = QString::fromLocal8Bit(file.readAll().data());
            lastVersion = lastTouched;
            stylesheet += QLatin1Char('\n') + userSheet;
            file.close();
        }
    }

    // The dir="auto" is required for WebKit to treat all paragraphs as entities with possibly different text direction.
    // The individual paragraphs unfortunately share the same text alignment, though, as per
    // https://bugs.webkit.org/show_bug.cgi?id=71194 (fixed in Blink already).
    QString htmlHeader(QLatin1String("<html><head><style type=\"text/css\"><!--") + textColors + fontSpecification + stylesheet +
                       QLatin1String("--></style></head><body><pre dir=\"auto\">"));
    static QString htmlFooter(QLatin1String("\n</pre></body></html>"));

    // We cannot rely on the QWebFrame's toPlainText because of https://bugs.kde.org/show_bug.cgi?id=321160
    QString markup = Composer::Util::plainTextToHtml(plainText, flowedFormat);
    return htmlHeader + markup + htmlFooter;
}

}
