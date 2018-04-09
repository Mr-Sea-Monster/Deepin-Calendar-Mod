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

#include "calendarwindow.h"
#include "dbuscalendar_adaptor.h"
#include "constants.h"
#include "infoview.h"

#include <QDate>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QWheelEvent>
#include <QPainter>
#include <QMenu>
#include <DTitlebar>
#include <DAboutDialog>
#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

static const int CalendarHeaderHeight = 60;

static const int CalendarWidth = 760;
static const int CalendarHeight = 500;

static const int InfoViewWidth = CalendarWidth - 86;
static const int InfoViewHeight = 90;

static const int ContentLeftRightPadding = 80;

static const int MinYearValue = 1900;

CalendarWindow::CalendarWindow() :
    DMainWindow(nullptr)
{
    setContentsMargins(QMargins(0, 0, 0, 0));

    m_settings = new QSettings;
    m_dateSettings = new QSettings("deepin", "dde-dock-datetime", this);

    initUI();
    initAnimation();
    initDateChangeMonitor();


    new CalendarAdaptor(this);
}

void CalendarWindow::handleTodayButtonClicked()
{
    m_calendarView->setCurrentDate(QDate::currentDate());
}

void CalendarWindow::handleCurrentYearMonthChanged(int year, int month)
{
    QDate changedDate;
    changedDate.setDate(year, month,  1);

    const uint daysInMonth = changedDate.daysInMonth();
    const uint currentDay = QDate::currentDate().day();
    if (currentDay > daysInMonth) {
        changedDate = changedDate.addDays(daysInMonth - 1);
    } else {
        changedDate = changedDate.addDays(currentDay - 1);
    }

    m_calendarView->blockSignals(true);
    m_calendarView->setCurrentDate(changedDate);
    m_calendarView->blockSignals(false);
}

void CalendarWindow::previousMonth()
{
    slideMonth(false);
}

void CalendarWindow::nextMonth()
{
    slideMonth(true);
}

void CalendarWindow::wheelEvent(QWheelEvent * e)
{
    if (e->delta() < 0) {
        nextMonth();
    } else {
        if (m_infoView->year() > MinYearValue || m_infoView->month() != 1) {
            previousMonth();
        }
    }
}

void CalendarWindow::initUI()
{
    m_contentBackground = new QFrame;
    m_contentBackground->setObjectName("CalendarBackground");
    m_contentBackground->setStyleSheet("QFrame#CalendarBackground { "
                             "background: rgba(0,0,0,0);"
                             "background-image: url(:/resources/calendar_skin.png);"
                             "}");
    m_contentBackground->setFixedSize(CalendarWidth + ContentLeftRightPadding * 2,
                                      InfoViewHeight + CalendarHeight);

    DTitlebar *titlebar = this->titlebar();
    const int titlebarHeight = titlebar ? titlebar->height() : 0;

    setAttribute(Qt::WA_TranslucentBackground);
    setTranslucentBackground(true);
    setEnableBlurWindow(true);

    setFixedSize(m_contentBackground->width(),
                 m_contentBackground->height() + titlebarHeight);

    m_icon = new QLabel(this);
    m_icon->setFixedSize(24, 24);
    m_icon->setPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-calendar.svg")
                      .scaled(m_icon->size() * devicePixelRatioF()));
    m_icon->move(12, 8);

    m_infoView = new InfoView;
    m_infoView->setStyleSheet("QFrame { background: rgba(0, 0, 0, 0) }");
    m_infoView->setFixedSize(InfoViewWidth, InfoViewHeight);
    m_infoView->setYearRange(MinYearValue, INT_MAX);

    m_infoView->setYear(QDate::currentDate().year());
    m_infoView->setMonth(QDate::currentDate().month());

    m_calendarView = new CalendarView;
    m_calendarView->setFixedSize(CalendarWidth, CalendarHeight);
    m_calendarView->setCurrentDate(QDate::currentDate());
    m_calendarView->setLunarVisible(QLocale::system().name().contains("zh"));

    m_calendarView->setFirstWeekday(m_settings->value("weekday").toInt());

    m_animationContainer = new QFrame(m_contentBackground);
    m_animationContainer->setStyleSheet("QFrame { background: rgba(0, 0, 0, 0) }");
    m_animationContainer->setFixedSize(m_calendarView->width(),
                                       m_calendarView->height() - CalendarHeaderHeight);
    m_animationContainer->move(ContentLeftRightPadding, CalendarHeaderHeight + InfoViewHeight);
    m_animationContainer->hide();

    m_fakeContent = new QLabel(m_animationContainer);
    m_fakeContent->setStyleSheet("QLabel { background: rgba(0, 0, 0, 0) }");
    m_fakeContent->setFixedSize(m_animationContainer->width(),
                                m_animationContainer->height() * 2);

    QVBoxLayout * contentLayout = new QVBoxLayout(m_contentBackground);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_infoView, 0, Qt::AlignHCenter);
    contentLayout->addWidget(m_calendarView, 0, Qt::AlignHCenter);
    contentLayout->addWidget(m_contentBackground);

    setCentralWidget(m_contentBackground);

    connect(m_calendarView, &CalendarView::currentDateChanged, [this](int year, int month){
        qDebug() << "current date changed" << year << month;
        m_infoView->blockSignals(true);
        m_infoView->setYear(year);
        m_infoView->setMonth(month);
        m_infoView->blockSignals(false);
    });
    connect(m_calendarView, &CalendarView::currentFestivalChanged, m_infoView, &InfoView::setFestival);
    connect(m_infoView, &InfoView::todayButtonClicked,
            this, &CalendarWindow::handleTodayButtonClicked);

    connect(m_infoView, &InfoView::yearChanged, [this](int year) {
        const int month = m_infoView->month();
        handleCurrentYearMonthChanged(year, month);
    });
    connect(m_infoView, &InfoView::monthChanged, [this](int month) {
        const int year = m_infoView->year();
        handleCurrentYearMonthChanged(year, month);
    });

    setupMenu();
}

void CalendarWindow::initAnimation()
{
    m_scrollAnimation = new QPropertyAnimation(m_fakeContent, "pos");
    m_scrollAnimation->setDuration(300);

    connect(m_scrollAnimation, &QPropertyAnimation::finished, [this]{
        m_animationContainer->hide();
    });
}

void CalendarWindow::initDateChangeMonitor()
{
    static QDate LastCurrentDate = QDate::currentDate();
    updateDate();
    updateTime();

    QTimer * timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [this] {
        QDate currentDate = QDate::currentDate();
        if (LastCurrentDate != currentDate) {
            LastCurrentDate = currentDate;
            updateDate();
        }
        updateTime();
    });
    timer->start();
}

void CalendarWindow::setupMenu()
{
    DTitlebar *titlebar = this->titlebar();

    if (titlebar) {
        titlebar->setWindowFlags(titlebar->windowFlags() & ~Qt::WindowMaximizeButtonHint);
        titlebar->setMenu(new QMenu(titlebar));
        titlebar->setSeparatorVisible(true);

        QMenu *firstWeekday = titlebar->menu()->addMenu(tr("First Day of Week"));
        QLocale locale;

        m_monAction = firstWeekday->addAction(locale.dayName(1, QLocale::ShortFormat));
        m_tueAction = firstWeekday->addAction(locale.dayName(2, QLocale::ShortFormat));
        m_wedAction = firstWeekday->addAction(locale.dayName(3, QLocale::ShortFormat));
        m_thuAction = firstWeekday->addAction(locale.dayName(4, QLocale::ShortFormat));
        m_friAction = firstWeekday->addAction(locale.dayName(5, QLocale::ShortFormat));
        m_satAction = firstWeekday->addAction(locale.dayName(6, QLocale::ShortFormat));
        m_sunAction = firstWeekday->addAction(locale.dayName(7, QLocale::ShortFormat));

        connect(titlebar->menu(), &QMenu::triggered, this, &CalendarWindow::menuItemInvoked);
    }
}


void CalendarWindow::menuItemInvoked(QAction *action)
{

    if (action == m_monAction) {
        m_calendarView->setFirstWeekday(Monday);
        m_settings->setValue("weekday", Monday);
        return;
    }

    if (action == m_tueAction) {
        m_calendarView->setFirstWeekday(Tuesday);
        m_settings->setValue("weekday", Tuesday);
        return;
    }

    if (action == m_wedAction) {
        m_calendarView->setFirstWeekday(Wednesday);
        m_settings->setValue("weekday", Wednesday);
        return;
    }

    if (action == m_thuAction) {
        m_calendarView->setFirstWeekday(Thursday);
        m_settings->setValue("weekday", Thursday);
        return;
    }

    if (action == m_friAction) {
        m_calendarView->setFirstWeekday(Friday);
        m_settings->setValue("weekday", Friday);
        return;
    }

    if (action == m_satAction) {
        m_calendarView->setFirstWeekday(Saturday);
        m_settings->setValue("weekday", Saturday);
        return;
    }

    if (action == m_sunAction) {
        m_calendarView->setFirstWeekday(Sunday);
        m_settings->setValue("weekday", Sunday);
        return;
    }
}

void CalendarWindow::slideMonth(bool next)
{
    m_animationContainer->show();
    m_animationContainer->raise();

    QPixmap one = getCalendarSnapshot();
    m_infoView->increaseMonth(next);
    QPixmap two = getCalendarSnapshot();
    QPixmap target = next ? joint(one, two) : joint(two, one);
    m_fakeContent->setPixmap(target);

    m_scrollAnimation->setStartValue(QPoint(0, next ? 0 : -one.height()));
    m_scrollAnimation->setEndValue(QPoint(0, next ? -one.height() : 0));

    m_scrollAnimation->start();
}

QPixmap CalendarWindow::getCalendarSnapshot() const
{
    return m_calendarView->grab(m_calendarView->rect().adjusted(0, CalendarHeaderHeight, 0, 0));
}

QPixmap CalendarWindow::joint(QPixmap &top, QPixmap &bottom) const
{
    QPixmap target(qMax(top.width(), bottom.width()),
                   top.height() + bottom.height());

    target.fill(Qt::black);
    QPainter painter;
    painter.begin(&target);
    painter.drawPixmap(0, 0, top);
    painter.drawPixmap(0, top.height(), bottom);
    painter.end();

    return target;
}

void CalendarWindow::updateTime() const
{
    m_infoView->setTime(QDateTime::currentDateTime().toString(m_dateSettings->value("24HourFormat", true).toBool() ? "hh:mm" : "hh:mm A"));
}

void CalendarWindow::updateDate() const
{
    QDate currentDate = QDate::currentDate();
    m_calendarView->setCurrentDate(currentDate);

    DTitlebar *titlebar = this->titlebar();
    if (titlebar) {
        titlebar->setTitle(currentDate.toString(Qt::SystemLocaleLongDate));
    }
}
