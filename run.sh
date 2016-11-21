#!/bin/bash

nohup mpirun --bind-to none -np 5 ./test_mpicall > nohup &
