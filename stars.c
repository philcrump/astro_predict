#include "stdint.h"
#include "math.h"

#include "stars.h"


#define SIDEREAL_DAY_HOURS  24.0 // SOLAR DAY: 23.9344696
// Note that the seconds here are sidereal rather than SI.  One
//  sidereal second is about 0.99727 SI seconds.

#define RAD2DEG(x)  (((x) * 180.0) / M_PI)
#define DEG2RAD(x)  (((x) / 180.0) * M_PI)

#define RAD2HR(x)   (((x) * SIDEREAL_DAY_HOURS) / (2 * M_PI))
#define HR2RAD(x)   (((x) / SIDEREAL_DAY_HOURS) * (2 * M_PI))

#define DEG2HR(x)   (((x) / 360.0) * SIDEREAL_DAY_HOURS)

/* Sourced from http://ned.ipac.caltech.edu/ */
const radiostar_t radiostar_taua = 
{
    .mean_ra_rad = HR2RAD(05. + 34./60. + 31.97/3600.),
    .mean_dec_rad = DEG2RAD(22. + 00./60. + 52.1/3600.),
    .pm_ra_rad_per_year = 0.0,
    .pm_dec_rad_per_year = 0.0,
    .parallax_arcsec = 0.0,
    .radial_vel_kmpersec = 0.0,
    .epoch_equinox_julian = 2000.0 // J2000 = 2000.0
};
const radiostar_t radiostar_cassa = 
{
    .mean_ra_rad = HR2RAD(23. + 23./60. + 27.94/3600.),
    .mean_dec_rad = DEG2RAD(58. + 48./60. + 42.4/3600.),
    .pm_ra_rad_per_year = 0.0,
    .pm_dec_rad_per_year = 0.0,
    .parallax_arcsec = 0.0,
    .radial_vel_kmpersec = 0.0,
    .epoch_equinox_julian = 2000.0 // J2000 = 2000.0
};
const radiostar_t radiostar_cygnusa = 
{
    .mean_ra_rad = HR2RAD(19. + 59./60. + 28.36/3600.),
    .mean_dec_rad = DEG2RAD(40. + 44./60. + 02.1/3600.),
    .pm_ra_rad_per_year = 0.0,
    .pm_dec_rad_per_year = 0.0,
    .parallax_arcsec = 0.0,
    .radial_vel_kmpersec = 0.0,
    .epoch_equinox_julian = 2000.0 // J2000 = 2000.0
};
const radiostar_t radiostar_oriona = 
{
    .mean_ra_rad = HR2RAD(05. + 35./60. + 16.48/3600.),
    .mean_dec_rad = DEG2RAD(05. + 23./60. + 22.8/3600.),
    .pm_ra_rad_per_year = 0.0,
    .pm_dec_rad_per_year = 0.0,
    .parallax_arcsec = 0.0,
    .radial_vel_kmpersec = 0.0,
    .epoch_equinox_julian = 2000.0 // J2000 = 2000.0
};