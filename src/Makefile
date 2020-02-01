DEPDIR = .deps
DF = $(DEPDIR)/$(*F)
CC = gcc

CFLAGS = -O3 -DNDEBUG  -I. -Wall -pedantic
LFLAGS = -O3 -lauto -lraauto -lpng -lz -lmgl -lm

MAKEDEPEND = $(CC) -MM $(CFLAGS) -o $(DF).d $<

OBJS = glmatrix.o glmatrix_gui.o glmatrix_main.o glmatrix_prefs.o screen_blanker.o yarandom.o

SRCS = $(OBJS:%.o=%.c)

GLMatrix: $(OBJS)
	$(CC) -o $@ $(OBJS)  $(LFLAGS)
	strip -s $@

$(OBJS): %.o : %.c
	@$(MAKEDEPEND); \
	copy $(DF).d $(DF).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $(DF).d >> $(DF).P; \
	delete $(DF).d;
	$(CC) $(CFLAGS) -c $<

clean:
	delete #?.o $(DEPDIR)/#?.P $(DEPDIR)/#?.d GLMatrix QUIET

-include $(SRCS:%.c=$(DEPDIR)/%.P)
