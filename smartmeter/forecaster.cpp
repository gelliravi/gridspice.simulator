#include "forecaster.h"
#include <math.h>

forecaster::forecaster(TIMESTAMP start, size_t num_days, db_access *db_)
{
    base = round_to_day_start(start);
    // base = (start / S_DAY_IN_SECONDS) * S_DAY_IN_SECONDS; // mod to start of day
    db = db_;

    // TODO verify range on dates?

    // init params
    params.svm_type = EPSILON_SVR;
    params.kernel_type = RBF;
    params.gamma = 0;
    params.cache_size = 100;
    params.eps = 0.001;
    params.C = 1;
    params.p = 0.1;
    params.shrinking = 1;
    params.probability = 0;

    // update the model to contain these days
    curr_bound = base + (num_days * S_DAY_IN_SECONDS);
    update_training_data(base, curr_bound);

    for (int i = 0; i < S_NUM_DAILY_INTERVALS; i++) {
        svm_models[i].sp = NULL;
        svm_models[i].model = NULL;
    }
    update_models();

}

bool forecaster::update_forecast(TIMESTAMP bound)
{
    bound = round_to_day_start(bound);
    update_training_data(curr_bound, bound);
    curr_bound = bound;
    update_models();
}

double forecaster::predict_load(TIMESTAMP t, bool is_dr)
{
    datapoint *d = calculate_datapoint(t, is_dr);        
    svm_node *n = datapoint_to_node(d);

    // figure out interval
    DATETIME dt;
    gl_localtime(t, &dt);
    // TODO abstract out to smartmeter_time ?
    int interval_number = (dt.hour * 4) + (dt.minute / 15) + 1; // out of 96

    double to_return = svm_predict(svm_models[interval_number - 1].model, n); 
    free(d);
    free(n);

    return to_return;
}

void forecaster::update_training_data(TIMESTAMP start, TIMESTAMP bound)
{
    for (TIMESTAMP cur_day = start; cur_day < bound; 
         cur_day += S_DAY_IN_SECONDS) {
        for (int interval = 0; interval < S_NUM_DAILY_INTERVALS; interval++) {
            TIMESTAMP cur_time = cur_day + (interval * 60);
            datapoint *d = calculate_datapoint(cur_time);
            training_data[interval].data.push_back(d);           
        }
    }
}

/* static helper functions */
forecaster::datapoint *forecaster::calculate_datapoint(TIMESTAMP t)
{

    DATETIME dt;    
    gl_localtime(t, &dt);
    datapoint *d = calculate_datapoint(t, false);
    d->is_dr = db->is_dr (dt);
    
    return d;
}

forecaster::datapoint *forecaster::calculate_datapoint(TIMESTAMP t, bool is_dr)
{
    DATETIME dt;    
    gl_localtime(t, &dt);

    datapoint *d = new datapoint;
    d->humidity = db->get_humidity(dt);
    if (d->humidity != -1) {
        d->humidity = pow(d->humidity - 50, 2);
    }
    d->temp = db->get_temp(dt);
    if (d->temp != -1) {
        d->temp = pow(d->temp - 68, 2);
    }
    d->is_dr = is_dr;
    d->load = db->get_power_usage(dt);
    d->dt = dt;

    for (int i = 0; i < NUM_LOAD_HISTORY_DAYS; i++) {
        TIMESTAMP curr = t - ((i + 1) * S_DAY_IN_SECONDS);        
        DATETIME curr_dt;
        gl_localtime(curr, &curr_dt);
        d->load_history[i] = db->get_power_usage(curr_dt);
    }

    return d;
}


svm_node *forecaster::datapoint_to_node(datapoint *dp)
{
    std::vector<svm_node *> temp_vector;

    // humidity feature
    if (dp->humidity != -1) {
        svm_node *curr = new svm_node;
        curr->index = 1;
        curr->value = dp->humidity;
        temp_vector.push_back(curr);
    }

    // temp feature
    if (dp->temp != -1) {
        svm_node *curr = new svm_node;
        curr->index = 2;
        curr->value = dp->temp;
        temp_vector.push_back(curr);
    }

    // dr feature
    svm_node *curr = new svm_node;
    curr->index = 3;
    curr->value = (dp->is_dr) ? 1 : 0;
    temp_vector.push_back(curr);

    // previous history load features
    for (int i = 0; i < NUM_LOAD_HISTORY_DAYS; i++) {
        if (dp->load_history[i] != -1) {
            svm_node *curr = new svm_node;
            curr->index = 4 + i;
            curr->value = dp->load_history[i];
            temp_vector.push_back(curr);
        }
    }

    svm_node *final = new svm_node;
    final->index = -1;
    final->value = -1;
    temp_vector.push_back(final);

    // svm_node *to_return = new svm_node[temp_vector.size()];
    svm_node *to_return = (svm_node *) malloc(sizeof(svm_node) * temp_vector.size());
    for (int i = 0; i < temp_vector.size(); i++) {
        memcpy(&to_return[i], temp_vector[i], sizeof(svm_node));
        free(temp_vector[i]);
        // to_return[i] = (svm_node *) temp_vector[i];
    }
    return to_return;
}

svm_problem *forecaster::construct_problem(int interval)
{
    std::vector<forecaster::datapoint *> *data = &training_data[interval].data;
    svm_problem *sp = new svm_problem;
    sp->l = data->size();
    sp->y = new double[sp->l];
    sp->x = new svm_node *[sp->l];
    for (int i = 0; i < data->size(); i++) {
        datapoint *dp = data->at(i);
        sp->y[i] = dp->load;
        sp->x[i] = datapoint_to_node(dp);
    }
    return sp;
}

void forecaster::update_models()
{
    for (int i = 0; i < S_NUM_DAILY_INTERVALS; i++) {
        if (svm_models[i].sp != NULL) {
            free_svm_problem(svm_models[i].sp);
        }
        if (svm_models[i].model != NULL) {
            svm_free_model_content(svm_models[i].model);
            free(svm_models[i].model);
        }

        svm_models[i].sp = construct_problem(i); 
        const char *error = svm_check_parameter(svm_models[i].sp, &params);
        svm_models[i].model = svm_train(svm_models[i].sp, &params);
    }
}

void forecaster::free_svm_problem(svm_problem *sp)
{
    delete [] sp->y;
    for (int i = 0; i < sp->l; i++) {
        svm_node *curr = sp->x[i];
        free(curr);
    }
    delete [] sp->x;
    free(sp);
}
