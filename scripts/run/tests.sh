#!/bin/bash

docker compose run core /app/scripts/deploy/make.sh && docker compose run core /app/test $@
