/* MQ4 methane sensor functions
 * File: me_mq4.c
 * Started: Fri 20 Nov 2020 11:55:12 AM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2020 ..."
 */

#include <me_mq4.h>
#include <math.h>

#define MQ4_COMP_ERR(T,RH)   ((0.83 * ( T /10)*( T /10) - 9.2 * ( T /10) \
                              + 25 - 0.3 * RH) / 100)

#define MQ4_RSRO_PPM(R)      1000.0d * pow((1.0d/R ), (1.0d/0.36152689d))


static double mq4_rsro(double milliV, me_mq4_t *calib)
{
    double rsro;
    
    rsro = (calib->mV_ref / milliV - 1) / calib->divider;
    
    return rsro;
}

double me_mq4_ppm(double milliV, me_mq4_t *calib)
{
    double ppm, rsro;
    
    rsro = mq4_rsro(milliV, calib);
    
    ppm  = MQ4_RSRO_PPM(rsro);

    return ppm;
}

double me_mq4_ppm_compensated(double milliV, me_mq4_t *calib, double temp, double rh)
{
    double ppm, rsro;
    
    rsro = mq4_rsro(milliV, calib) - MQ4_COMP_ERR(temp, rh);
    
    ppm  = MQ4_RSRO_PPM(rsro);

    return ppm;
}

void me_mq4_calibrate(double refV, double measV, me_mq4_t *calib)
{
    calib->mV_ref   = refV;
    calib->divider  = (refV/measV - 1);
    calib->divider /= 4.4d;
}

void me_mq4_calibrate_compensated(double refV, double measV, me_mq4_t *calib, double temp, double rh)
{
    double rsro;

    calib->mV_ref  = refV;
    calib->divider = (refV/measV - 1);

    rsro = 1 - MQ4_COMP_ERR(temp, rh);

    calib->divider  = calib->divider / rsro;
    calib->divider /= 4.4d;
}
