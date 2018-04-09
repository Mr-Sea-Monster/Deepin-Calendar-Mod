#-------------------------------------------------
#
# Project created by QtCreator 2016-03-17T15:35:18
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-calendar
TEMPLATE = app
CONFIG += link_pkgconfig c++11
PKGCONFIG += dtkwidget

SOURCES += main.cpp\
    calendarview.cpp \
    calendarwindow.cpp \
    calendardbus.cpp \
    dbuscalendar_adaptor.cpp \
    infoview.cpp \
    spinner.cpp \
    weekindicator.cpp

HEADERS  += calendarwindow.h \
    calendardbus.h \
    calendarview.h \
    constants.h \
    dbuscalendar_adaptor.h \
    infoview.h \
    spinner.h \
    weekindicator.h

RESOURCES += \
    resources.qrc

# Automating generation .qm files from .ts files
!system($$PWD/translate_generation.sh): error("Failed to generate translation")
!system($$PWD/translate_desktop.sh): error("Failed to generate translation")

isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $$PREFIX/bin/

desktop_files.path = $$PREFIX/share/applications/
desktop_files.files = $$PWD/dde-calendar.desktop

icon_files.path = $$PREFIX/share/icons/hicolor/scalable/apps/
icon_files.files = $$PWD/resources/icon/dde-calendar.svg

qm_files.path = $$PREFIX/share/dde-calendar/translations/
qm_files.files = $$PWD/translations/*.qm

service.path = $$PREFIX/share/dbus-1/services/
service.files = $$PWD/com.deepin.Calendar.service

TRANSLATIONS = $$PWD/translations/dde-calendar.ts

INSTALLS += target desktop_files icon_files qm_files service
