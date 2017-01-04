include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a


ifeq ($(SERIAL_FLASH),YES)
OBJS	= $(BOARD).o ../common/ar7100_flash.o ../common/vsc8601_phy.o ../common/generic_spi.o ../common/vsc73xx.o ../ap83/flash.o
else
OBJS	= $(BOARD).o ../common/ar9100_pflash.o ../common/vsc8601_phy.o ../common/generic_spi.o ../common/vsc73xx.o
endif
SOBJS	= ../common/lowlevel_init_ar9100.o

$(LIB):	.depend $(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
