/*
* drop2imageshack.cpp
* Copyright (C) 2009 Alexander Saltykov
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*/

#include "drop2imageshack.h"

#include <KIcon>
#include <KNotification>
#include <Plasma/Svg>
#include <Plasma/IconWidget>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneDragDropEvent>
#include <QRegExp>
#include <QStringList>
#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>

#include <QFile>

static void notify(const QString& msg)
{
    KNotification::event("image-link", msg, QPixmap(), 0,
                         KNotification::CloseOnTimeout,
                         KComponentData("plasma-drop2imageshack", "plasma-drop2imageshack", KComponentData::SkipMainComponentRegistration));
}

static bool is_valid_file(const QString& f)
{
    QStringList acceptable;
    acceptable << "jpg" << "jpeg" << "png" << "bmp" << "tiff" << "tif";
    if ( !acceptable.contains(f.split(".").last().toLower()) ) {
        qDebug("file %s is not accept\able", qPrintable(f));
        return false;
    }
    if ( !QFile::exists(f) ) {
        return false;
    }
    notify( QString("%1 is valid file").arg(f) );
    return true;
}

static size_t cb_write(void *ptr, size_t size, size_t nmemb, void *plasmoid)
{
    PlasmaIS *p = static_cast<PlasmaIS*>(plasmoid);

    size_t sz = size*nmemb+1;
    char *xml = new char[sz];
    qMemSet(xml, 0, sz);
    qMemCopy(xml, ptr, sz-1);

    QString s(xml);

    delete[] xml;

    QRegExp rx("<image_link>(.*)</image_link>");
    int pos = rx.indexIn(s);
    if ( pos >= 0 ) {
        QString value = rx.cap(1);
        qDebug("url is: %s", qPrintable(value));
        p->setLastUrl(value);
    }
    return size*nmemb;
}

PlasmaIS::PlasmaIS(QObject *parent, const QVariantList& args)
    : Plasma::Applet(parent, args)
{
    m_svg = new Plasma::Svg(this);
    m_icon = new Plasma::IconWidget("", this);

    curl_global_init(0);
}

PlasmaIS::~PlasmaIS()
{
    curl_global_cleanup();
    delete m_icon;
    delete m_svg;
}

void PlasmaIS::init()
{
    m_svg->setImagePath("widgets/background");

    setHasConfigurationInterface(false);
    setBackgroundHints(DefaultBackground);
    setAspectRatioMode(Plasma::ConstrainedSquare);

    setAcceptDrops(true);

    QGraphicsLinearLayout *l = new QGraphicsLinearLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->setOrientation(Qt::Horizontal);
    m_icon->setIcon( KIcon("image-loading") );
    l->addItem(m_icon);
    resize( m_icon->size() );

    connect( m_icon, SIGNAL(clicked()), SLOT(slotScreenshot()) );
}

void PlasmaIS::setLastUrl(const QString& url)
{
    m_url = url;
}

void PlasmaIS::dragEnterEvent(QGraphicsSceneDragDropEvent *e)
{
    if ( e->mimeData()->hasFormat("text/plain") ) {
        e->accept();
    } else {
        e->ignore();
    }
}

void PlasmaIS::dropEvent(QGraphicsSceneDragDropEvent *e)
{
    QString addr = e->mimeData()->text();
    addr.replace("file://", "");
    if ( !is_valid_file(addr) ) {
        notify("Invalid image file");
        return;
    }
    upload(addr);
}

void PlasmaIS::upload(const QString& f)
{
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "fileupload",
                 CURLFORM_FILE, f.toLocal8Bit().constData(), CURLFORM_END);
    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "xml",
                 CURLFORM_COPYCONTENTS, "yes", CURLFORM_END);

    CURL *h = curl_easy_init();
    curl_easy_setopt(h, CURLOPT_URL, "http://www.imageshack.us/index.php");
    curl_easy_setopt(h, CURLOPT_POST, 1);
    curl_easy_setopt(h, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, cb_write);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, this);

    // for debugging purposes
    curl_easy_setopt(h, CURLOPT_VERBOSE, 1);

    CURLcode c = curl_easy_perform(h);
    curl_formfree(post);

    if ( c != CURLE_OK ) {
        notify("Curl error occured");
    }

    if ( !m_url.isEmpty() ) {
        notify( QString("Upload success: %1\nURL was copied to clipboard").arg(m_url) );
        QApplication::clipboard()->setText(m_url);
        m_url.clear();
    }

    curl_easy_cleanup(h);
}

void PlasmaIS::slotScreenshot()
{
    // TODO: Remove depedency from scrot

    QString f = "/tmp/plasma-drop2imageshack" + QString::number(qrand()) + ".png";
    qDebug("filename is: %s", qPrintable(f));

    if ( system( QString("scrot "+f).toLocal8Bit().constData() ) == EXIT_SUCCESS ) {
        upload(f);
        remove( f.toLocal8Bit().constData() );
    }
}

#include "drop2imageshack.moc"
