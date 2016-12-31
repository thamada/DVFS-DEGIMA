#!/bin/bash

#Engine clock (min,max,step): (8000, 120000, 500)
#Memory clock (min,max,step): (15000, 140000, 500)
#Core voltage (min,max,step): (1062, 1212, 5)

#Adaptor DEFAULT performance level values:
#	level 0 (Engine clock, Memory clock, Core voltage): (15700,30000,1062)
#	level 1 (Engine clock, Memory clock, Core voltage): (60000,90000,1112)
#	level 2 (Engine clock, Memory clock, Core voltage): (85000,120000,1212)


echo ''; echo '------------------- 1st STAGE--------------------------'
./bcast.pl '/export/opt/src/sample-radeon/tweak/monitor_gpu/bin/adl_cpl 0   0 8000 15000 1107 ' ./hosts 
./bcast.pl '/export/opt/src/sample-radeon/tweak/monitor_gpu/bin/adl_cpl 0   1 8000 15000 1207 ' ./hosts 
./bcast.pl '/export/opt/src/sample-radeon/tweak/monitor_gpu/bin/adl_cpl 0   2 8000 15000 1212 ' ./hosts 

echo ''; echo '------------------- 2nd STAGE--------------------------'
./bcast.pl '/export/opt/src/sample-radeon/tweak/monitor_gpu/bin/adl_cpl 0   1 8000 15000 1212 ' ./hosts 
./bcast.pl '/export/opt/src/sample-radeon/tweak/monitor_gpu/bin/adl_cpl 0   0 8000 15000 1212 ' ./hosts 

