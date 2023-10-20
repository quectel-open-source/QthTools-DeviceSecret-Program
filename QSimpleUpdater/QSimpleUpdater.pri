QT += gui
QT += core
QT += network
QT += widgets

DEFINES += QSU_INCLUDE_MOC=1
INCLUDEPATH += $$PWD/include

SOURCES += \
    $$PWD/help/about.cpp \
    $$PWD/src/Updater.cpp \
    $$PWD/src/Downloader.cpp \
    $$PWD/src/QSimpleUpdater.cpp \
    $$PWD/toolupdate.cpp

HEADERS += \
    $$PWD/help/about.h \
    $$PWD/include/QSimpleUpdater.h \
    $$PWD/src/Updater.h \
    $$PWD/src/Downloader.h \
    $$PWD/toolupdate.h \
    $$PWD/include/updateConfig.h

FORMS += $$PWD/src/Downloader.ui \
        $$PWD/help/about.ui \
        $$PWD/toolupdate.ui

RESOURCES += $$PWD/etc/resources/qsimpleupdater.qrc \
    $$PWD/language.qrc

TRANSLATIONS += \
    $$PWD/language/chinese.ts \
    $$PWD/language/english.ts
