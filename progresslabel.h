/*
 * progresslabel.h
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
#ifndef PROGRESSLABEL_H_
#define PROGRESSLABEL_H_

#include <QGraphicsWidget>

class ProgressLabel : public QGraphicsWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ProgressLabel)

    public:
        ProgressLabel(QGraphicsWidget *parent = 0);
    public slots:
        void setProgress(double percent);
    protected:
        void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget* w);

        void resizeEvent(QGraphicsSceneResizeEvent *e);
    private:
        void init_paint_options();
    private:
        QFont m_font;
        QColor m_textColor;
        QColor m_boxColor;

        int m_progress;
};

#endif /* PROGRESSLABEL_H_ */
