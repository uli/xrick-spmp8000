LIBSPMP8K = ../..

TARGET	= xrick

OBJS = src/unzip.o src/data.o src/scr_xrick.o src/scr_pause.o src/scr_imain.o src/scr_imap.o src/scr_gameover.o src/scr_getname.o src/dat_picsPC.o src/dat_picsST.o src/dat_screens.o src/dat_tilesPC.o src/dat_tilesST.o src/dat_maps.o src/dat_ents.o src/dat_spritesST.o src/dat_spritesPC.o src/ents.o src/e_bullet.o src/e_bomb.o src/e_rick.o src/e_sbonus.o src/e_them.o src/e_bonus.o src/e_box.o src/rects.o src/util.o src/game.o src/xrick.o src/draw.o src/maps.o src/sysvid.o src/syskbd.o src/control.o src/system.o src/scroller.o src/sysevt.o src/sysarg.o src/syssnd.o src/sysjoy.o src/dat_snd.o

LIBS	= -lz

include $(LIBSPMP8K)/main.cfg
include $(LIBGAME)/libgame.mk

CFLAGS += -Iinclude -O2 -W -Wall

include/version.h: .git/index
	build_no=`git rev-list HEAD | wc -l | sed -e 's/ *//g' | xargs -n1 printf %d`.`git show HEAD|head -n1|cut -c8-11|tr a-z A-Z`; \
	echo "#define BUILD_STRING \"$$build_no\"" > $@
