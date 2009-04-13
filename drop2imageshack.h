/*
 * drop2imageshack.h
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

#ifndef DROP2IMAGESHACK_H_
#define DROP2IMAGESHACK_H_

#include <Plasma/Applet>

namespace Plasma {
    class Svg;
    class IconWidget;
}

class PlasmaIS : public Plasma::Applet
{
    Q_OBJECT

    public:
        PlasmaIS(QObject *parent, const QVariantList& args);
        ~PlasmaIS();

        void init();
        void setLastUrl(const QString& url);
    protected:
        void dragEnterEvent(QGraphicsSceneDragDropEvent *e);
        void dropEvent(QGraphicsSceneDragDropEvent *e);
    private:
        void upload(const QString& f);
    private slots:
        void slotScreenshot();
    private:
        Plasma::Svg *m_svg;
        Plasma::IconWidget *m_icon;
        QString m_url;
};

K_EXPORT_PLASMA_APPLET(drop2imageshack, PlasmaIS);

#endif /* DROP2IMAGESHACK_H_ */
