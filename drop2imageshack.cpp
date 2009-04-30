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
#include "imageuploader.h"

#include <KIcon>
#include <KNotification>
#include <KRun>
#include <KUrl>
#include <Plasma/IconWidget>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneDragDropEvent>
#include <QStringList>
#include <stdio.h>
#include <stdlib.h>

static void notify_error(const QString& msg)
{
    KNotification::event(KNotification::Error, msg, QPixmap(), 0,
                         KNotification::CloseOnTimeout);
}

static bool is_valid_file(const QString& f)
{
    QStringList acceptable;
    acceptable << "jpg" << "jpeg" << "png" << "bmp" << "tiff" << "tif";
    if ( !acceptable.contains(f.split(".").last().toLower()) ) {
        return false;
    }
    if ( !QFile::exists(f) ) {
        return false;
    }
    return true;
}

PlasmaIS::PlasmaIS(QObject *parent, const QVariantList& args)
    : Plasma::Applet(parent, args),
    m_icon(0),
    m_uploader(0),
    m_notify(0)
{
    setAcceptDrops(true);
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(false);
    resize(128, 128);
}

PlasmaIS::~PlasmaIS()
{
    delete m_icon;
    delete m_notify;
    delete m_uploader;
}

void PlasmaIS::init()
{
    m_icon = new Plasma::IconWidget( KIcon("image-loading"), QString(), this);
    m_icon->setToolTip( i18n("Drop an image or click this icon to upload") );

    QGraphicsLinearLayout *l = new QGraphicsLinearLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setOrientation(Qt::Vertical);
    l->setSpacing(0);

    l->addItem(m_icon);

    connect( m_icon, SIGNAL(clicked()), SLOT(slotScreenshot()) );
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
        notify_error( i18n("'%1' is invalid image file").arg(addr) );
        return;
    }
    upload(addr);
}

void PlasmaIS::upload(const QString& f)
{
    if ( m_uploader ) { // uploader object exists => already uploading.
        return;
    }
    m_uploader = new ImageUploader;

    QObject::connect( m_uploader, SIGNAL(curlError(QString)),
                      SLOT(slotCurlError(QString)) );
    QObject::connect( m_uploader, SIGNAL(imageUploaded(QString)),
                      SLOT(slotImageUploaded(QString)) );
    QObject::connect( m_uploader, SIGNAL(finished()),
                      SLOT(slotUploaderFinished()) );

    m_uploader->setUploadFile(f);
    m_uploader->start();
}

void PlasmaIS::slotScreenshot()
{
    // TODO: Remove depedency from scrot

    if ( m_uploader ) { // uploader object exists => already uploading.
        return;
    }

    m_tmpscr = "/tmp/plasma-drop2imageshack" + QString::number(qrand()) + ".png";

    int result = system( QString("scrot "+m_tmpscr).toLocal8Bit().constData() );
    if ( result == EXIT_SUCCESS ) {
        upload(m_tmpscr);
    } else {
        notify_error( i18n("scrot failed with code: %1").arg(QString::number(result)) );
    }
}

void PlasmaIS::slotCurlError(const QString& errDesc)
{
    notify_error(errDesc);
}

void PlasmaIS::slotImageUploaded(const QString& url)
{
    QApplication::clipboard()->setText(url);
    m_lasturl = url;

    delete m_notify; // FIXME: KDE hides persistent notifications and doesn't clean the memory.
    m_notify = new KNotification("image-link", 0, KNotification::Persistent);
    m_notify->setActions( QStringList(i18n("Open Browser")) );
    m_notify->setComponentData( KComponentData("plasma-drop2imageshack",
                                               "plasma-drop2imageshack",
                                               KComponentData::SkipMainComponentRegistration) );
    connect( m_notify, SIGNAL(action1Activated()),
             SLOT(slotOpenUrl()) );
    m_notify->setText( i18n("Upload success: %1").arg(url) +
                       QString(QChar::ParagraphSeparator) +
                       i18n("URL was copied to clipboard") );
    m_notify->sendEvent();
}

void PlasmaIS::slotUploaderFinished()
{
    m_uploader->deleteLater();
    m_uploader = 0;

    if ( !m_tmpscr.isEmpty() ) {
        remove( m_tmpscr.toLocal8Bit().constData() );
        m_tmpscr.clear();
    }
}

void PlasmaIS::slotOpenUrl()
{
    m_notify->close();
    m_notify = 0;
    new KRun(KUrl(m_lasturl), 0);
}

#include "drop2imageshack.moc"
