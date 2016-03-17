!isEmpty(QMAKE_LIBDIR_EGL): LIBS += -L$$QMAKE_LIBDIR_EGL
LIBS += $$QMAKE_LIBS_EGL
QT -= gui core
CONFIG += c++11
use_khr_headers {
    message("Using internal Khronos EGL headers.")
    INCLUDEPATH += $$PWD/3rdparty/khronos
} else {
    message("Using system EGL headers.")
    INCLUDEPATH += $$QMAKE_INCDIR_EGL
}
SOURCES += main.cpp
