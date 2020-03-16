QT += widgets
requires(qtConfig(treeview))

FORMS       =
HEADERS     = mainwindow.h \
              treemodel.h \
              treenode.h
RESOURCES   = resource.qrc
SOURCES     = mainwindow.cpp \
              treemodel.cpp \
              main.cpp \
              treenode.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/itemviews/editabletreemodel
INSTALLS += target
