OpenMesh OM5P-AC v2 Unlocker
============================

Here’s our current build status: [![CircleCI](https://circleci.com/gh/true-systems/om5p-ac-v2-unlocker/tree/master.svg?style=svg&circle-token=158c0fd70915de6831e446ab0eec1c685f3bfc55)](https://circleci.com/gh/true-systems/om5p-ac-v2-unlocker/tree/master)

How to use it?
--------------

1. Setup a static IP **192.168.100.8/24** on your PC network card.
2. Configure and start any **TFTP** (not FTP!) server with **option negotiation mechanism** enabled.
3. Download and place inside TFTP server directory two files: **fwupgrade.cfg** and **fwupgrade.cfg.sig**.
4. Connect, using a LAN cable, **OM5P-AC v2** with your PC. Use RJ45 socket near the DC jack only (labeled as **18-24v POE**).
5. Power up **OM5P-AC v2** and observe its LEDs.
6. During files download, which should not take more than 2 minutes, **second blue LED** will blink.
7. At the end, if everything goes well, the **green LED** will start blinking (1 Hz), otherwise the **red** one will.
8. If nothing happens at all, check your TFTP server configuration, firewall on PC, etc. Let us know if you see that the device downloads both exploit files from the server, but nothing happens after that or router simply reboots.

After successful unlock (blinking green LED), you can power down device. Inside the TFTP server directory you will find a backup of **ART** (**Atheros Radio Test**) data partition, named **om5p-ac-v2__MAC_backup.bin**. From this point, your **OM5P-AC v2** will accept any alternative firmware.
