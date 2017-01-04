include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	= $(BOARD).o flash.o ../common/vsc_phy.o ../common/ipPhy.o
SOBJS	= ../common/lowlevel_init.o

$(LIB):	.depend $(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
