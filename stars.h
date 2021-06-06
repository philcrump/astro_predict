#ifndef __STARS_H__
#define __STARS_H__

#include <stdint.h>

typedef struct {
    double mean_ra_rad;
    double mean_dec_rad;
    double pm_ra_rad_per_year;
    double pm_dec_rad_per_year;
    double parallax_arcsec;
    double radial_vel_kmpersec; // (+ve if receding)
    double epoch_equinox_julian; // J2000 = 2000.0
} radiostar_t;

extern const radiostar_t radiostar_taua;
extern const radiostar_t radiostar_cassa;
extern const radiostar_t radiostar_cygnusa;
extern const radiostar_t radiostar_oriona;

#endif /* __STARS_H__ */