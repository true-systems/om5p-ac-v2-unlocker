#ifndef _M88E1116_PHY_H
#define _M88E1116_PHY_H

/*****************/
/* PHY Registers */
/*****************/
#define M88E1116_PHY_CONTROL                 0
#define M88E1116_PHY_STATUS                  1
#define M88E1116_PHY_ID1                     2
#define M88E1116_PHY_ID2                     3
#define M88E1116_AUTONEG_ADVERT              4
#define M88E1116_LINK_PARTNER_ABILITY        5
#define M88E1116_AUTONEG_EXPANSION           6
#define M88E1116_NEXT_PAGE_TRANSMIT          7
#define M88E1116_LINK_PARTNER_NEXT_PAGE      8
#define M88E1116_1000BASET_CONTROL           9
#define M88E1116_1000BASET_STATUS            10
#define M88E1116_PHY_SPEC_CONTROL            16
#define M88E1116_PHY_SPEC_STATUS             17

/* M88E1116_PHY_CONTROL fields */
#define M88E1116_CTRL_SOFTWARE_RESET                    0x8000
#define M88E1116_CTRL_SPEED_LSB                         0x2000
#define M88E1116_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define M88E1116_CTRL_RESTART_AUTONEGOTIATION           0x0200
#define M88E1116_CTRL_SPEED_FULL_DUPLEX                 0x0100
#define M88E1116_CTRL_SPEED_MSB                         0x0040

#define M88E1116_RESET_DONE(phy_control)                   \
    (((phy_control) & (M88E1116_CTRL_SOFTWARE_RESET)) == 0)
/* Phy status fields */
#define M88E1116_STATUS_AUTO_NEG_DONE                   0x0020
#define M88E1116_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (M88E1116_STATUS_AUTO_NEG_DONE)) ==                    \
        (M88E1116_STATUS_AUTO_NEG_DONE))
/* Link Partner ability */
#define M88E1116_LINK_100BASETX_FULL_DUPLEX       0x0100
#define M88E1116_LINK_100BASETX                   0x0080
#define M88E1116_LINK_10BASETX_FULL_DUPLEX        0x0040
#define M88E1116_LINK_10BASETX                    0x0020
/* Advertisement register. */
#define M88E1116_ADVERTISE_NEXT_PAGE              0x8000
#define M88E1116_ADVERTISE_ASYM_PAUSE             0x0800
#define M88E1116_ADVERTISE_PAUSE                  0x0400
#define M88E1116_ADVERTISE_100FULL                0x0100
#define M88E1116_ADVERTISE_100HALF                0x0080
#define M88E1116_ADVERTISE_10FULL                 0x0040
#define M88E1116_ADVERTISE_10HALF                 0x0020
#define M88E1116_ADVERTISE_ALL (M88E1116_ADVERTISE_10HALF | M88E1116_ADVERTISE_10FULL | \
                            M88E1116_ADVERTISE_100HALF | M88E1116_ADVERTISE_100FULL)
/* 1000BASET_CONTROL */
#define M88E1116_ADVERTISE_1000FULL               0x0200
/* Phy Specific status fields */
#define M88E1116_STATUS_LINK_MASK                 0xC000
#define M88E1116_STATUS_LINK_SHIFT                14
#define M88E1116_STATUS_FULL_DEPLEX               0x2000
#define M88E1116_STATUS_LINK_PASS                 0x0400
#define M88E1116_STATUS_RESOVLED                  0x0800

int m88e1116_phy_is_up(int unit);
int m88e1116_phy_is_fdx(int unit);
int m88e1116_phy_speed(int unit);
int m88e1116_phy_setup(int unit);

typedef enum {
        AG7100_PHY_SPEED_10T,
        AG7100_PHY_SPEED_100TX,
        AG7100_PHY_SPEED_1000T,
}ag7100_phy_speed_t;

#endif /* _M88E1116_PHY_H */
