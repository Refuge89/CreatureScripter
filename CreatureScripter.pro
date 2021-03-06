#-------------------------------------------------
#
# Project created by QtCreator 2017-04-15T15:03:42
#
#-------------------------------------------------

QT += core widgets sql gui xml webenginewidgets webengine concurrent

TARGET = CreatureScripter
TEMPLATE = app
CONFIG += debug_and_release
CONFIG += c++11
DEFINES += __STORMLIB_SELF__ QSW_LIB ELYSIUM #CMANGOS

INCLUDEPATH += $$PWD/mpq/StormLib
DEPENDPATH += $$PWD/mpq/StormLib

#win32:RC_ICONS += resources/mangos.ico

RESOURCES += \
    icons.qrc \
    css.qrc \
    json.qrc

INCLUDEPATH += src/ \
src/CreatureTabs/ \
src/ScriptAI/ \
src/ScriptAI/widgets/ \

HEADERS += $$files(src/*.h) \
    src/collapsableframe.h \
    qswwrapper.h \
    src/ScriptAI/widgets/flagswidget.h \
    src/ScriptAI/widgets/clickablewidget.h \
    src/ScriptAI/eventaidefelysium.h \
    src/ScriptAI/eventaidefcmangos.h \
    src/ScriptAI/widgets/inversephasemaskwidget.h \
    src/ScriptAI/eventaidefcommon.h \
    src/ScriptAI/changeswidget.h \
    src/ScriptAI/widgets/boolwidget.h \
    src/ScriptAI/widgets/conditionwidget.h \
    src/ScriptAI/widgets/eventtypewidget.h \
    src/loadingscreen.h \
    src/ScriptAI/statstemplate.h \
    src/creaturetemplatedef.h \
    src/ScriptAI/widgets/stringwidget.h
HEADERS += $$files(src/CreatureTabs/*.h)
HEADERS += $$files(src/ScriptAI/*.h)
HEADERS += $$files(src/ScriptAI/widgets/*.h)
HEADERS += $$files(src/Tables/*.h)

SOURCES += $$files(src/*.cpp) \
    src/collapsableframe.cpp \
    qswwrapper.cpp \
    src/ScriptAI/widgets/flagswidget.cpp \
    src/ScriptAI/widgets/inversephasemaskwidget.cpp \
    src/ScriptAI/eventaidefcommon.cpp \
    src/ScriptAI/changeswidget.cpp \
    src/ScriptAI/widgets/boolwidget.cpp \
    src/ScriptAI/widgets/conditionwidget.cpp \
    src/ScriptAI/widgets/eventtypewidget.cpp \
    src/loadingscreen.cpp \
    src/ScriptAI/statstemplate.cpp \
    src/creaturetemplatedef.cpp \
    src/ScriptAI/widgets/stringwidget.cpp
SOURCES += $$files(src/CreatureTabs/*.cpp)
SOURCES += $$files(src/ScriptAI/*.cpp)
SOURCES += $$files(src/ScriptAI/widgets/*.cpp)
SOURCES += $$files(src/Tables/*.cpp)


SOURCES += \
    AboutForm.cpp \
    Alphanum.cpp \
    MainForm.cpp \
    ScriptEdit.cpp \
    SettingsForm.cpp \
    blp/blp.cpp \
    dbc/DBC.cpp \
    mpq/MPQ.cpp \
    wov/bone.cpp \
    wov/camerashake.cpp \
    wov/m2.cpp \
    wov/model.cpp \
    wov/modelscene.cpp \
    wov/mvp.cpp \
    wov/particleemitter.cpp \
    wov/ribbonemitter.cpp \
    wov/spellvisual.cpp \
    wov/spellvisualkit.cpp \
    wov/texture.cpp \
    wov/textureanimation.cpp \
    wov/wovdbc.cpp \
    mustache/mustache.cpp \
    models.cpp \
    qsw.cpp \
    spellwork.cpp \
    wov/wov.cpp

HEADERS  += \
    AboutForm.h \
    Alphanum.h \
    MainForm.h \
    ScriptEdit.h \
    SettingsForm.h \
    blp/blp.h \
    dbc/DBC.h \
    mpq/MPQ.h \
    wov/animatedvalue.h \
    wov/bone.h \
    wov/camerashake.h \
    wov/m2.h \
    wov/m2structures.h \
    wov/model.h \
    wov/modelscene.h \
    wov/mvp.h \
    wov/particleemitter.h \
    wov/ribbonemitter.h \
    wov/spellvisual.h \
    wov/spellvisualkit.h \
    wov/texture.h \
    wov/textureanimation.h \
    wov/wovdbc.h \
    mustache/mustache.h \
    models.h \
    events.h \
    qsw.h \
    spellwork.h \
    wov/wov.h

FORMS += \
    about.ui \
    main.ui \
    settings.ui \
    scriptFilter.ui

OTHER_FILES += \
    wov/particle.fs \
    wov/particle.vs \
    wov/shader.fs \
    wov/shader.vs

RESOURCES += \
    qsw.qrc \
    wov/wov.qrc

defineTest(copyToDestdir) {
    files = $$1

    for(FILE, files) {

        DDIR = $$DESTDIR\\$$2

        win32:FILE ~= s,/,\\,g
        win32:DDIR ~= s,/,\\,g

        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}

win32: {
    contains(QT_ARCH, i386) {
        PLATFORM = "Win32"
    } else {
        PLATFORM = "x64"
    }
    CONFIG(debug, debug|release) {
        BUILDTYPE = "Debug"
    } else {
        BUILDTYPE = "Release"
    }

    LIBS += -L$$PWD/mpq/StormLib/$$PLATFORM/$$BUILDTYPE/ -lStormLib
    LIBS += -L$$PWD/blp/squish/$$PLATFORM/$$BUILDTYPE/ -lsquish
    DLLDESTDIR = $$OUT_PWD/bin/$$PLATFORM/$$BUILDTYPE/
    DESTDIR = $$DLLDESTDIR

    copyToDestdir($$PWD/mpq/StormLib/$$PLATFORM/$$BUILDTYPE/StormLib.dll)

    QMAKE_POST_LINK += windeployqt --no-system-d3d-compiler --no-opengl-sw --no-svg --no-qmltooling $${DESTDIR}$${TARGET}.exe
}

