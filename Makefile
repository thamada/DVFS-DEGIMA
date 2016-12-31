ADL_INCLUDE=${PWD}/ADL_SDK/include
OUTPUT_DIR=bin
CPPFLAGS=-g

all: adl_ls adl_qa adl_cpl adl_temp

adl_temp:
	gcc adl_monitor_temperature.c -o bin/adl_temp  -DLINUX -ldl -I /export/opt/src/sample-radeon/tweak/monitor_gpu/ADL_SDK/include -latiadlxx

adl_cpl:	
	gcc adl_change_performance_levels.c ${CPPFLAGS} -o ${OUTPUT_DIR}/adl_cpl -DLINUX -ldl -I ${ADL_INCLUDE}

adl_qa:
	gcc adl_query_adapter.c ${CPPFLAGS} -o ${OUTPUT_DIR}/adl_qa -DLINUX -ldl -I ${ADL_INCLUDE}

adl_ls:
	gcc adl_ls.c ${CPPFLAGS} -o ${OUTPUT_DIR}/adl_ls -DLINUX -ldl -I ${ADL_INCLUDE}

clean:
	rm ${OUTPUT_DIR}/adl_qa ${OUTPUT_DIR}/adl_ls ${OUTPUT_DIR}/adl_cpl

#---------------------------------------------------------------
list:
	./bin/adl_ls

stat:
	./bin/adl_qa 0

change:
	@echo 'adl_cpl changes performance levels of the GPU to values within the operational range reported for a device (see range using adl_qa).'
	@echo 'Command description:'
	@echo './bin/adl_cpl <device> <level> <engine clock> <memory clock> <voltage>'
	@echo 'Command example:'
	./bin/adl_cpl 0 1 15700 30000 1062

default:
	@echo 'Restoring performance values of device 0 to default for all levels.'
	@echo './bin/adl_cpl <device> <level> <engine clock> <memory clock> <voltage>'
	./bin/adl_cpl 0 0 15700 30000 1062
	./bin/adl_cpl 0 1 60000 90000 1112
	./bin/adl_cpl 0 2 85000 120000 1212

#	Engine clock (min,max,step): (8000, 120000, 500)
#	Memory clock (min,max,step): (15000, 140000, 500)
#	Core voltage (min,max,step): (1062, 1212, 5)

safe:
	@echo 'Restoring performance values of device 0 to default for all levels.'
	@echo './bin/adl_cpl <device> <level> <engine clock> <memory clock> <voltage>'
	./bin/adl_cpl 0 0 8000 15000 1212
	./bin/adl_cpl 0 1 8000 15000 1212
	./bin/adl_cpl 0 2 8000 15000 1212

