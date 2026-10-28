#pragma once
#include "src/lib/error.hpp"
#include <flux/core.h>
#include <jansson.h>

/**
 * Converts a job structure to a JSON object
 *
 * @param job The job structure to convert
 * @return A JSON object representing the job
 */
json_t *ps_job_to_json(const ps_job_t &job);

/**
 * Converts a JSON object to a job structure
 *
 * @param obj The JSON object to convert
 * @return A result containing the converted job
 */
ps_result_t<ps_job_t> ps_job_from_json(json_t *obj);
