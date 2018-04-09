/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QFrame>

#include <dlinkbutton.h>

DWIDGET_USE_NAMESPACE

class QLabel;
class Spinner;

class InfoView : public QFrame
{
    Q_OBJECT
public:
    explicit InfoView(QFrame *parent = 0);

signals:
    void yearChanged(int year);
    void monthChanged(int month);
    void todayButtonClicked();

public slots:
    void setTime(const QString &time) const;
    void setFestival(const QString &festival) const;

    int year() const;
    int month() const;
    void setYear(int year) const;
    void setMonth(int month) const;
    void setYearRange(int min, int max) const;

    void increaseMonth(bool increase);

    void setTodayButtonVisible(bool visible) const;

private:
    QLabel * m_timeLabel;
    QLabel * m_festivalLabel;
    DLinkButton * m_todayButton;
    Spinner * m_yearSpinner;
    Spinner * m_monthSpinner;
};

#endif // INFOVIEW_H
