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
#include <Plasma/Svg>
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
    : Plasma::Applet(parent, args)
{
    m_svg = new Plasma::Svg(this);
    m_icon = new Plasma::IconWidget("", this);
    m_icon->setToolTip("Drop an image or click this icon to upload");

    m_uploader = 0;
}

PlasmaIS::~PlasmaIS()
{
    delete m_icon;
    delete m_svg;
    delete m_notify;
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

    m_notify = new KNotification("image-link", 0, KNotification::Persistent);
    m_notify->setActions( QStringList(i18n("Open Browser")) );
    m_notify->setComponentData( KComponentData("plasma-drop2imageshack",
                                               "plasma-drop2imageshack",
                                               KComponentData::SkipMainComponentRegistration) );
    connect( m_notify, SIGNAL(action1Activated()),
             SLOT(slotOpenUrl()) );
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
    if ( m_uploader ) {
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

    if ( m_uploader ) {
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
    new KRun(KUrl(m_lasturl), 0);
}

#include "drop2imageshack.moc"
