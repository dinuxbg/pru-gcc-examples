#!/bin/sh

RPROC0=/sys/class/remoteproc/remoteproc1
RPROC1=/sys/class/remoteproc/remoteproc2

echo stop > $RPROC0/state
echo stop > $RPROC1/state

# Inputs
config-pin P9_31 pruin	# pru0_pru_r31_0
config-pin P9_29 pruin	# pru0_pru_r31_1
config-pin P9_30 pruin	# pru0_pru_r31_2
config-pin P9_28 pruin	# pru0_pru_r31_3

config-pin P9_27 pruin	# pru0_pru_r31_5

config-pin P9_25 pruin	# pru0_pru_r31_7

# Outputs
config-pin P2_24 pruout	# pru0_pru_r30_14
config-pin P2_33 pruout	# pru0_pru_r30_15

cp out/pru-core0.elf /lib/firmware/
cp out/pru-core1.elf /lib/firmware/
echo pru-core0.elf > $RPROC0/firmware
echo pru-core1.elf > $RPROC1/firmware

echo start > $RPROC0/state
echo start > $RPROC1/state
