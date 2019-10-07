//
// Created by Rocío Parra on 10/5/2019.
//

#include "board_ev_sources.h"
#include "board_can_network.h"
#include "board_database.h"
#include <string.h>
#include <stdlib.h>
#include "../Accelerometer/accelerometer.h"
#include <math.h>
#include "../util/vector_3d.h"
#include "../util/clock.h"

#define THRESHOLD	5

#define RAD2DEG(x)  ((x)*180.0/M_PI)

#define MAX2(a,b)   ((a)>(b) ? (a) : (b))
#define MAX3(a,b,c) MAX2( (a), MAX2((b), (c)) )

void can_callback(uint8_t msg_id, uint8_t * can_data);

static int32_t curr_angles[N_ANGLE_TYPES];
static clock_t last;

#define ACC_MAX_FREQ    100
#define ACC_MIN_MS      (1000.0/ACC_MAX_FREQ)



void get_angles(int32_t * angles);
void must_update_angles(ivector_t ang_end, ivector_t ang_start, bool * ans);
int round2(double x);


void be_init()
{
    static bool is_init = false;
    if (is_init)
        return;
    is_init = true;

    bd_init();

    bn_init();
    bn_register_callback(can_callback);
    // initialize magnetometer & accelerometer

    clock_init();
    last = get_clock();
}

void be_periodic()
{
	static bool acc_init = false;
	if (!acc_init) {
	    accel_init();
	}
	acc_init = true;

    bn_periodic();

    clock_t now = get_clock();
    if (1000.0*(now - last)/(float)CLOCKS_PER_SECOND >= ACC_MIN_MS) {
        int32_t new_angles[N_ANGLE_TYPES];
        get_angles(new_angles);
        bool updates[N_ANGLE_TYPES];
        ivector_t old = {curr_angles[0], curr_angles[1], curr_angles[2]};
        ivector_t new = {new_angles[0], new_angles[1], new_angles[2]};

        must_update_angles(old, new, updates);

        unsigned int i;
        for (i = 0; i < N_ANGLE_TYPES; i++) {
        	if (updates[i]) {
        		bd_update(1, i, new_angles[i]);
        	}
        }
    }

}

void can_callback(uint8_t msg_id, uint8_t * can_data)
{
    if (can_data != NULL)
    {
        uint32_t len = strlen(can_data);
        if (len <= MAX_LEN_CAN_MSG && len >= 2) { // at least angle type and one number
            angle_type_t type;

            switch (can_data[0]) {
                case PITCH_CHAR:    type = PITCH;           break;
                case ROLL_CHAR:     type = ROLL;            break;
                case OR_CHAR:       type = ORIENTATION;     break;
                default:            type = N_ANGLE_TYPES;   break; // error in msg
            }

            if (type <= N_ANGLE_TYPES) {
                uint8_t * end;
                int32_t value = (int32_t)strtol(&can_data[1], &end, 10);
                if (end[0] == 0) {
                    bd_update(msg_id, type, value);
                }
            }
        }
    }
}

void get_angles(int32_t * angles)
{
    accel_raw_data_t accel = accel_get_last_data(ACCEL_ACCEL_DATA);
    accel_raw_data_t magn = accel_get_last_data(ACCEL_MAGNET_DATA);

    vector_t g = {-(float)accel.x, -(float)accel.y, -(float)accel.z};
    vector_t b = {(float)magn.x, (float)magn.y, (float)magn.z};

    vector_t u = v_invert(v_normalize(g)); // u = -G/norm(G);

    vector_t n = v_substract(b, v_scalar_product(v_dot_product(b, u), u)); // n = B - dot(B,u)*u;
    n = v_normalize(n); // n = n/norm(n);

    vector_t e = v_cross_product(n, u);


    if (fabs((double)u.y) >= 1 ) {
        u.y = (u.y > 0 ? 1 : -1); // restrict to [-1, 1]
        angles[ORIENTATION] = -round2(RAD2DEG(atan2((double)n.x, (double)e.x)));
        angles[ROLL] = 0;
    }
    else {
        angles[ORIENTATION] = -round2(RAD2DEG(atan2(-(double)e.y, (double)n.y)));
        angles[ROLL] = round2(RAD2DEG(atan2(-(double)u.x, (double)u.z)));
    }

    angles[PITCH] = round2(RAD2DEG(asin((double)u.y)));
}


void must_update_angles(ivector_t ang_end, ivector_t ang_start, bool * ans)
{
    // write end angle in all possible formats
    // ang_end_alt_1 = [ang_end(1),-ang_end(2),ang_end(3)]+[180,180,-180];
    ivector_t ang_end_alt_1 = {ang_end.x+180.0,-ang_end.y+180,ang_end.z-180};
    // ang_end_alt_2 = [ang_end(1),-ang_end(2),ang_end(3)]+[-180,180,180];
    ivector_t ang_end_alt_2 = {ang_end.x-180,-ang_end.y+180,ang_end.z+180};
    ivector_t ang_end_alts[3] = {ang_end, ang_end_alt_1, ang_end_alt_2};

    unsigned int i, min = 0;
    float min_norm = 10000;

    for(i = 0; i < 3; i++) {
        float norm = iv_norm(iv_substract(ang_start, ang_end_alts[i]));
        if (norm <= min_norm) {
            min_norm = norm;
            min = i;
        }
    }

    ivector_t diff = iv_substract(ang_end_alts[min], ang_start);

    ans[0] = abs(diff.x) >= THRESHOLD;
    ans[1] = abs(diff.y) >= THRESHOLD;
    ans[2] = abs(diff.z) >= THRESHOLD;

    ivector_t sum = iv_add(ang_start, diff);
    if (sum.x == ang_end.x && sum.y == ang_end.y && sum.z == ang_end.z) {
        ans[0] = ans[1] = ans[2] = true;
    }
}



int round2(double x)
{
    if (x < 0.0)
        return (int)(x - 0.5);
    else
        return (int)(x + 0.5);
}

