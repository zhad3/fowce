#-------------------------------------------------
#
# Project created by QtCreator 2018-08-26T10:54:45
#
#-------------------------------------------------

QT       += core gui widgets xml svg

TARGET = FOWCE
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    util.cpp \
    card.cpp \
    cardpreviewitem.cpp \
    cardpreviewpainter.cpp \
    cardpreviewtextitem.cpp \
    cardpreviewwidget.cpp \
    dialogs/addcardsidedialog.cpp \
    dialogs/addtraitdialog.cpp \
    fowce.cpp \
    langstringdelegate.cpp \
    langstringlistmodel.cpp \
    logger.cpp \
    mainwindow.cpp \
    models/fabstractobject.cpp \
    models/fabstractyamlmodel.cpp \
    models/fattributemodel.cpp \
    models/fcardtypemodel.cpp \
    models/fgeneralcardtypemodel.cpp \
    models/flanguagemodel.cpp \
    models/flanguagestring.cpp \
    models/fraritymodel.cpp \
    models/fwillcharacteristicmodel.cpp \
    text/ftextobject.cpp \
    widgets/attributecheckboxgroup.cpp \
    widgets/buttonlineedit.cpp \
    widgets/cardsidepushbutton.cpp \
    widgets/fcombobox.cpp \
    widgets/multilanglineedit.cpp \
    widgets/multilangtextedit.cpp \
    willcostdelegate.cpp \
    willcostmodel.cpp \
    text/fgraphicstextitem.cpp \
    text/ftextdocumentlayout.cpp \
    text/ftextcursor.cpp \
    dialogs/optionswindow.cpp \
    dialogs/languagestringeditdialog.cpp



HEADERS += \
    util.h \
    card.h \
    cardpreviewitem.h \
    cardpreviewpainter.h \
    cardpreviewtextitem.h \
    cardpreviewwidget.h \
    dialogs/addcardsidedialog.h \
    dialogs/addtraitdialog.h \
    langstringdelegate.h \
    langstringlistmodel.h \
    logger.h \
    mainwindow.h \
    models/fabstractobject.h \
    models/fabstractyamlmodel.h \
    models/fattributemodel.h \
    models/fcardtypemodel.h \
    models/fgeneralcardtypemodel.h \
    models/flanguagemodel.h \
    models/flanguagestring.h \
    models/fraritymodel.h \
    models/fwillcharacteristicmodel.h \
    models/yamlconvert.h \
    text/ftextobject.h \
    widgets/attributecheckboxgroup.h \
    widgets/buttonlineedit.h \
    widgets/cardsidepushbutton.h \
    widgets/fcombobox.h \
    widgets/multilanglineedit.h \
    widgets/multilanglineeditwidget.h \
    widgets/multilangtextedit.h \
    willcostdelegate.h \
    willcostmodel.h \
    qfixed_p.h \
    text/fgraphicstextitem.h \
    text/ftextdocumentlayout.h \
    text/ftextcursor.h \
    dialogs/optionswindow.h \
    dialogs/languagestringeditdialog.h


FORMS += \
        cardpreviewwidget.ui \
        mainwindow.ui \
    dialogs/addtraitdialog.ui \
    dialogs/addcardsidedialog.ui \
    widgets/attributecheckboxgroup.ui \
    dialogs/multilanglineeditdialog.ui \
    dialogs/optionswindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc \
    icons.qrc

unix|win32: LIBS += -L$$PWD/../build-yaml-cpp-Desktop_Qt_5_12_2_MSVC2017_64bit-Debug/ -llibyaml-cppmdd

INCLUDEPATH += $$PWD/../yaml-cpp/include
DEPENDPATH += $$PWD/../yaml-cpp/include
