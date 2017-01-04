include $(TOPDIR)/config.mk

LIB	= lib$(SOC).a

START	=
#OBJS	= ar7100_serial.o ag7100.o ar7100_flash.o meminit.o
OBJS	= ar7100_serial.o ag7100.o meminit.o
SOBJS	=

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS) $(OBJS-0)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
