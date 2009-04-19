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
#include <Plasma/Svg>
#include <Plasma/IconWidget>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneDragDropEvent>
#include <QStringList>
#include <stdio.h>
#include <stdlib.h>

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
        notify( QString("Scrot failed with code: %1").arg(QString::number(result)) );
    }
}

void PlasmaIS::slotCurlError(const QString& errDesc)
{
    notify(errDesc);
}

void PlasmaIS::slotImageUploaded(const QString& url)
{
    notify( QString("Upload success: %1").arg(url)+QString(QChar::ParagraphSeparator)+QString("URL was copied to clipboard") );
    QApplication::clipboard()->setText(url);
    qDebug("upload success");
}

void PlasmaIS::slotUploaderFinished()
{
    qDebug("thread finished");
    m_uploader->deleteLater();
    m_uploader = 0;

    if ( !m_tmpscr.isEmpty() ) {
        remove( m_tmpscr.toLocal8Bit().constData() );
        m_tmpscr.clear();
    }
}

#include "drop2imageshack.moc"
