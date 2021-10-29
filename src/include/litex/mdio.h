#ifndef __MDIO_H
#define __MDIO_H

void mdio_write(int phyadr, int reg, int val);
int mdio_read(int phyadr, int reg);

#endif
