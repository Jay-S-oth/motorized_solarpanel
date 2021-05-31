#include <FindSun.h>

#define PEAKVALUE 350

// init the sensor
void Find_Sun::init_compass()
{
    compass.setReset();
    compass.init();
    endPos.init_sensors(D6, D5);
    compass.setMode(Mode_Continuous, ODR_10Hz, RNG_8G, OSR_512);
    SunPos.init_time();
    pinMode(A0, INPUT);
    tiltpanel.init_motor(D7, D4, 2);
    turnTable.init_motor(D0, D3, 1);
}
// gets the azimuth value of the sensor
int Find_Sun::get_current_azimuth()
{
    compass.read();
    this->current_azimuth = compass.getAzimuth();
    return current_azimuth;
}

// calc the offset to sun then sent to the motors
int Find_Sun::offset_to_Sun()
{
    Sun_at_azimuth = SunPos.get_azimuth(SunPos.get_arr_pos());
    offset = current_azimuth - Sun_at_azimuth;
    return offset;
}
//
void Find_Sun::check_tilt()
{
    endPos.read_pins();
    int light_intens;
    light_intens = analogRead(A0);
    int time_var = SunPos.get_arr_pos();
    if (time_var >= 19 && time_var <= 41)  // time betwee 9:30 and 15:00 -> Adustable time
    {
        if (light_intens > PEAKVALUE)  // light inensity not enough
        {
            if (endPos.getPosPhiUp() == 1 && endPos.getPosPhiDown() == 1) //Pos=Tilt Max
            {
                tiltpanel.switch_pwr(OFF);
                tiltpanel.rotate(DOWN);

                while (endPos.getPosPhiDown() == 1 && light_intens > PEAKVALUE) // If light intensity is to low and upper end is reched -> search down
                {
                    endPos.read_pins();
                    light_intens = analogRead(A0);
                    turnTable.switch_pwr(OFF);
                    delay(10);
                }
            }
            else if (endPos.getPosPhiUp() == 0 && endPos.getPosPhiDown() == 0) // Pos=Tilt Min
            {
                tiltpanel.switch_pwr(OFF);
                tiltpanel.rotate(UP);
                while (endPos.getPosPhiUp() == 0 && light_intens > PEAKVALUE) // If light intensity is to low and down end is reched -> search up
                {
                    endPos.read_pins();
                    light_intens = analogRead(A0);
                    turnTable.switch_pwr(OFF);
                    delay(10);
                }
            }
            else if (endPos.getPosPhiUp() == 0 && endPos.getPosPhiDown() == 1)
            {
                if (time_var >= 34)  // if light intensity is lost and its before true noon -> search up
                {
                    tiltpanel.switch_pwr(OFF);
                    tiltpanel.rotate(UP);
                }
                else
                {
                    tiltpanel.switch_pwr(OFF); // if light intensity is lost and its after true noon -> search down
                    tiltpanel.rotate(DOWN);
                }
            }
        }
        else
        {
            delay(500); // delay stop driving after light intensity reached to go into better tolerance
            tiltpanel.switch_pwr(OFF);
        }
    }
    else // Time before and after adustable time -> Tilt goto Max
    {
        if (endPos.getPosPhiUp() == 1)
        {
            tiltpanel.switch_pwr(OFF);
        }
        else
        {
            tiltpanel.switch_pwr(OFF);
            tiltpanel.rotate(UP);
        }
    }
}

int Find_Sun::check_rotation()
{
    SunPos.get_azimuth(SunPos.get_arr_pos()); //update target angle
    get_current_azimuth(); //update current angle

    if (offset_to_Sun() < -TOLERANCE) //Angle is not ok
    {
        turnTable.rotate(RIGHT);
        return 1;
    }
    else if (offset_to_Sun() > TOLERANCE) //Angle is not ok
    {
        turnTable.rotate(LEFT);
        return 1;
    }

    else if (offset_to_Sun() >= TOLERANCE || offset_to_Sun() >= -TOLERANCE) //Angle is ok
    {
        turnTable.switch_pwr(OFF);
        return 0;
    }
    return 1;
}
