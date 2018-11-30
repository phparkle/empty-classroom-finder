QT       += core gui widgets network concurrent
TARGET = empty-classroom-finder
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    global.cpp \
    lib/htmlcxx/html/Extensions.cc \
    lib/htmlcxx/html/Node.cc \
    lib/htmlcxx/html/ParserDom.cc \
    lib/htmlcxx/html/ParserSax.cc \
    lib/htmlcxx/html/Uri.cc \
    lib/htmlcxx/html/utils.cc \
    lib/hcxselect/hcxselect.cpp \
    httpclient.cpp \
    emptyclassroomfinder.cpp \
    address.cpp

HEADERS += \
    mainwindow.h \
    global.h \
    lib/htmlcxx/html/ci_string.h \
    lib/htmlcxx/html/debug.h \
    lib/htmlcxx/html/Extensions.h \
    lib/htmlcxx/html/Node.h \
    lib/htmlcxx/html/ParserDom.h \
    lib/htmlcxx/html/ParserSax.h \
    lib/htmlcxx/html/tld.h \
    lib/htmlcxx/html/tree.h \
    lib/htmlcxx/html/Uri.h \
    lib/htmlcxx/html/utils.h \
    lib/hcxselect/hcxselect.h \
    lib/hcxselect/lexer.h \
    httpclient.h \
    emptyclassroomfinder.h \
    address.h \
    weektime.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/lib
