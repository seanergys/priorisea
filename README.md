# PrioriSea

Seanergys' priority plugin for Flux.

/!\ This is work in progress and the information written below are 
objectives, not the current state of the project.

_(Name is not definitive)_

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

## Tests

The testing is separated into:
- TAP tests using sharness (user-side tests)
- Unit tests per language (software-side tests)
