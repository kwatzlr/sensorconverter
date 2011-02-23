ROOTINCDIR = $$system(root-config --incdir)
!exists ($$ROOTINCDIR/TObject.h):error("Could NOT find ROOT!")

include($$ROOTINCDIR/rootcint.pri)

LIBS += $$system(root-config --cflags --libs)
LIBS += -lHistPainter
