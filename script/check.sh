#!/bin/bash

echo ''
echo '-------------- LEVEL 0 ---------------'

m.do '~/monitor_gpu/bin/adl_qa 0 |grep Level' ./hosts |grep 'Level 0'

echo ''
echo '-------------- LEVEL 1 ---------------'
m.do '~/monitor_gpu/bin/adl_qa 0 |grep Level' ./hosts |grep 'Level 1'

echo ''
echo '-------------- LEVEL 2 ---------------'
m.do '~/monitor_gpu/bin/adl_qa 0 |grep Level' ./hosts |grep 'Level 2'






