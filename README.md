# PrioriSea

Seanergys' priority plugin for Flux.

## Philosophy

This plugin is designed to be integrated inside a Flux priority pipeline.
This mainly means that flux-accounting can be integrated separately to PrioriSea,
and users may design a final priority taking into account both flux-accounting's fairshare
as well as PrioriSea's energy scoring system.

## Components

PrioriSea includes two main components:
- a Flux job-manager plugin to assign priorities to jobs at submission time
- CLI commands to interface with Seanergys components (CMI, AIDAS, Database) and the plugin. This includes:
  + editing scoring formulas 
  + editing priority formulas
  + adding recurring time ranges for giving bonuses/maluses to scores
  + computing new jobs' scores
  + updating users score
  + editing the exponential decay of scores
  + and more...

### Commands

__Prefix__: `priorisea <subcommand>`

#### Subcommands:

__Init__

Initialize the database and plugin with default settings.

Always run this the first time after starting flux and loading the flux plugin.

- `init` _initialize database and plugin with default settings_

__Formulas__

A formula is a [tinyexpr](https://github.com/codeplea/tinyexpr) expression.

When evaluated, some formulas can set some variables values (ex: `job_priority` formula can set the `prev_priority` variable). To use them, pass the `--var` option with the variable name and default value (in case the variable is not set when evaluating the formula)

Example: `formula set job_priority "prev_priority * 2" --var prev_priority 5`

- `formula list` _list formulas_
- `formula get <formula_name1> <formula_name2> ...` _get formulas_
- `formula get <formula_name1> <formula_name2> ... --json` _get formula as array of {"name": "some_name", "formula": "some_formula"}_
- `formula set <formula_name> "<formula>" --var var_name var_value` _edit formula_
- `formula reset <formula_name1> <formula_name2> ...` _reset formulas_

__Time ranges__

A cron expression is a [croncpp](https://github.com/mariusbancila/croncpp) expression.

Cron expressions are used to define time ranges. The recurring windows last for `duration` seconds each time the cron expression is evaluated.

- `timewin list` _list time windows_
- `timewin get <range_name1> <range_name2> ...` _get time windows_
- `timewin add <range_name> --cron "<cron_expression>" --duration <duration> --priority <bonus_or_malus_priority>` _add time window_
- `timewin edit <range_name> --cron "<cron_expression>" --duration <duration> --priority <bonus_or_malus_priority>` _edit time window_
- `timewin delete <range_name1> <range_name2> ...` _delete time window_

__Scores__
- `score update --new` _compute new jobs score_
- `score update <job_id1> <job_id2> ...` _compute jobs score_
- `score job <job_id1> <job_id2> ...` _retrieve informations about the jobs score_
- `score job <job_id1> <job_id2> ... --json` _retrieve informations about the jobs score as array of {"job_id": some_jobid, "score": some_score}_
- `score user <user_id1> <user_id2> ...` _retrieve informations about the users score_
- `score user <user_id1> <user_id2> ... --json` _retrieve informations about the user score as array of {"user_id": some_userid, "score": some_score}_

__Decay__

A decay expression is a list of `duration:weight` separated by `,`. `duration` is the length of
the time slot in seconds.

An example expression could be: `86400:1,604800:0.5,1209600:0.25`

Meaning jobs ran on the same day keep their scores, jobs ran the next seven days (between 1-8 days) have their score multiplied by 0.5, jobs ran the next seven days (between 9-16 days) have their score multiplied by 0.25

- `decay get` _get decay expression_
- `decay get --json` _get decay expression as a ordered json array of {"duration": some_duration, "weight": some_weight}_
- `decay set "<decay_expression>"` _set decay expression, containing time slots and weights_
- `decay reset` _reset decay expression to default value_

## Tests

The testing is separated into:
- TAP tests using sharness (user-side tests)
- Unit tests per language (TODO)
