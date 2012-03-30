#ifndef _smartmeter_time_h
#define _smartmeter_time_h

#include "gridlabd.h"

static const int S_INTERVAL_SIZE = 15; // in minutes
static const int S_NUM_DAILY_INTERVALS = 24 * (60 / S_INTERVAL_SIZE);
static const size_t S_DAY_IN_SECONDS = 24 * 60 * 60 * TS_SECOND;
static const size_t S_INTERVAL_IN_SECONDS = S_INTERVAL_SIZE * 60 * TS_SECOND;

static inline TIMESTAMP round_to_day_start (TIMESTAMP t) {
    return (t / S_DAY_IN_SECONDS) * S_DAY_IN_SECONDS;
}

#endif

