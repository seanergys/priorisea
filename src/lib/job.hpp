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
 * Converts a job structure to a JSON object, excluding the formula fields
 * (f_expr, defaults, values)
 *
 * @param job The job structure to convert
 * @return A JSON object representing the job
 */
json_t *ps_job_to_json_plain(const ps_job_t &job);

/**
 * Convert a vector of jobs to a JSON object
 *
 * @param jobs The vector of jobs to convert
 * @return A JSON object representing the jobs
 */
json_t *ps_jobs_to_json(const std::vector<ps_job_t> &jobs);

/**
 * Converts a JSON object to a job structure
 *
 * @param obj The JSON object to convert
 * @return A result containing the converted job
 */
ps_result_t<ps_job_t> ps_job_from_json(json_t *obj);

/**
 * Converts a JSON object to a vector of jobs
 *
 * @param obj The JSON object to convert
 * @return A result containing the converted jobs
 */
ps_result_t<std::vector<ps_job_t>> ps_jobs_from_json(json_t *obj);
