#ifndef _smartmeter_forecaster_h
#define _smartmeter_forecaster_h

#include <vector>
#include "db_access.h"
#include "gridlabd.h"
#include "smartmeter_time.h"
#include "svm.h"

class forecaster {
private:

    /* static variables for the module */
    static const int NUM_LOAD_HISTORY_DAYS = 4;

    /* struct definitions */
    struct datapoint {
        // FEATURES
        double humidity; // -1 if doesn't exist
        double temp; // -1 if doesn't exist
        bool is_dr;

        /* number of days of load history to use. excludes yesterday's load,
           since this is a day ahead forecast */
        double load_history[NUM_LOAD_HISTORY_DAYS];
        DATETIME dt;
        
        // LOAD VALUE (output)
        double load;
    };

    struct interval_dataset {
        std::vector<datapoint *> data;     
    };

    struct svm_info {
        struct svm_problem *sp;
        struct svm_model *model;
    };

    /* private variables */

    /* the base day that is our first day of training data */
    TIMESTAMP base;

    /* bound on day that we train until (not inclusive) */
    TIMESTAMP curr_bound;

    db_access *db;

    /* parameters for the svm problem */
    struct svm_parameter params;

    /* vector containing features/output for each day from [base, curr_bound) 
       one for each interval of the day */
    interval_dataset training_data[S_NUM_DAILY_INTERVALS];

    /* saved svm_problem and model information for each interval */
    svm_info svm_models[S_NUM_DAILY_INTERVALS];

    /* private helper functions */ 
    void update_training_data(TIMESTAMP start, TIMESTAMP bound);
    datapoint *calculate_datapoint(TIMESTAMP t);
    datapoint *calculate_datapoint(TIMESTAMP t, bool is_dr);
    svm_node *datapoint_to_node(datapoint *dp);
    svm_problem *construct_problem(int interval);
    void free_svm_problem(svm_problem *sp);
    void update_models();

public:
    /* Constructor
       
       - start: the first day that we train from 
       - num_days: the number of days to train on beginning from start */
    forecaster(TIMESTAMP start, size_t num_days, db_access *db_);

    /* Updates models to train from base (passed in as start in constructor) 
       to bound. In other words, trains on the range [base, bound) */
    bool update_forecast(TIMESTAMP bound); // update to date

    /* Predict what the load will be at TIMESTAMP t 
    
       right now, just takes into account whether dr or not, but realistically,
       should aslo take in other features - humidity, temp, etc. */
    double predict_load(TIMESTAMP t, bool is_dr); // takes in datetime which stores day and interval
};

#endif
