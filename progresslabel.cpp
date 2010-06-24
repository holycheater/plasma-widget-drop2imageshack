/*
 * progresslabel.cpp
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

#include "progresslabel.h"

#include <KGlobalSettings>
#include <Plasma/Theme>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>

ProgressLabel::ProgressLabel(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
    m_progress(0)
{
    setAcceptDrops(false);
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(false);
    init_paint_options();
}

void ProgressLabel::setProgress(double percent)
{
    m_progress = qRound(percent);
    update();
}

void ProgressLabel::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget* w)
{
    Q_UNUSED(o);
    Q_UNUSED(w);

    QString text = QString::number(m_progress).append("%");
    QFontMetrics fm(m_font);
    int textWidth = fm.width(text) + 8;

    QRectF cr = rect();

    QRect textRect = QRect(cr.left() + (cr.right() - textWidth) / 2,
                           cr.top() + ((cr.height() - fm.height()) / 2 * 0.9),
                           qMin((int)cr.width(), textWidth),
                           fm.height() * 1.2);


    float round_prop = textRect.width() / textRect.height();
    qreal round_radius = 35.0;

    p->save();

    p->setPen(m_boxColor);
    p->setBrush(m_boxColor);
    p->setFont(m_font);

    p->drawRoundedRect(textRect, round_radius / round_prop, round_radius,
                       Qt::RelativeSize);

    p->setPen(m_textColor);
    p->drawText(textRect, Qt::AlignCenter, text);

    p->restore();
}

void ProgressLabel::resizeEvent(QGraphicsSceneResizeEvent *e)
{
    init_paint_options();
    e->accept();
}

void ProgressLabel::init_paint_options()
{
    m_font = KGlobalSettings::generalFont();
    m_font.setWeight(QFont::Bold);
    m_font.setBold(true);
    m_font.setPixelSize( qMax(size().width() / 5, qreal(1.0)) );

    m_textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    m_boxColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    m_boxColor.setAlphaF(0.5);
}
