PROJ = CONFIML
PROJFILE = CONFIML.MAK
DEBUG = 0

PWBRMAKE  = pwbrmake
NMAKEBSC1  = set
NMAKEBSC2  = nmake
LINKER  = link
ILINK  = ilink
LRF  = echo > NUL
BIND  = bind
RC  = rc
IMPLIB  = implib
LFLAGS_G  =  /A:16
LFLAGS_D  =  /CO 
LFLAGS_R  =  NUL  
MAPFILE_D  = NUL
MAPFILE_R  = NUL
CC  = cl
CFLAGS_G  = /AM /W4 /G2 /Gw /Zp 
CFLAGS_D  = /Od /Zi /Gi$(PROJ).mdt /Gs
CFLAGS_R  = /Os /Gs
LLIBS_R  =  /NOD:MLIBCE MLIBCEW
LLIBS_D  =  /NOD:MLIBCE MLIBCEW
LLIBS_G  = LIBW.LIB oldnames.lib
ASM  = masm
AFLAGS_G  = /Mx /T
AFLAGS_D  = /Zi
BROWSE  = 0
CVFLAGS  =  /25 /F
RUNFLAGS  = c:\imlapps\config\develop\hardware.inf c:\windmill

DEF_FILE  = CONFIML.DEF
OBJS  = CFDEVDLG.obj CFMAIN.obj CFINIINF.obj CFSETDLG.obj CFUTIL.obj\
        CFADDDLG.obj
RESS  = CONFIML.res
SBRS  = CFDEVDLG.sbr CFMAIN.sbr CFINIINF.sbr CFSETDLG.sbr CFUTIL.sbr\
        CFADDDLG.sbr

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .sbr .res .obj .c .rc

CFDEVDLG.obj : CFDEVDLG.C CONFIML.H confdlg.h

CFDEVDLG.sbr : CFDEVDLG.C CONFIML.H confdlg.h

CFMAIN.obj : CFMAIN.C CONFIML.H

CFMAIN.sbr : CFMAIN.C CONFIML.H

CONFIML.res : CONFIML.RC CONFIML.H confdlg.h confiml.dlg

CFINIINF.obj : CFINIINF.C CONFIML.H

CFINIINF.sbr : CFINIINF.C CONFIML.H

CFSETDLG.obj : CFSETDLG.C CONFIML.H confdlg.h

CFSETDLG.sbr : CFSETDLG.C CONFIML.H confdlg.h

CFUTIL.obj : CFUTIL.C CONFIML.H

CFUTIL.sbr : CFUTIL.C CONFIML.H

CFADDDLG.obj : CFADDDLG.C CONFIML.H confdlg.h

CFADDDLG.sbr : CFADDDLG.C CONFIML.H confdlg.h


$(PROJ).bsc : $(SBRS)
        $(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(DEF_FILE) $(OBJS) $(RESS)
!IF $(DEBUG)
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
!IF $(DEBUG)
        $(LINKER) @$(PROJ).lrf
!ELSE
        $(LINKER) @$(PROJ).lrf
!ENDIF
        $(RC) -30 $(RESS) $@


.c.sbr :
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FR$@ $<
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FR$@ $<
!ENDIF

.rc.res :
        $(RC) /r $<

.c.obj :
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /Fo$@ $<
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
!ENDIF


run: $(PROJ).exe
        WIN $(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
        WIN CVW $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)
