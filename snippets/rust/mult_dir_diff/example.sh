#!/bin/bash

cargo run -- --format=html --outfile=out_tesdata.html -d testdata/1_left -d testdata/2_right -d testdata/3
#cargo run -- --format=html --outfile=out_usr_opt.html -d /usr -d /opt
