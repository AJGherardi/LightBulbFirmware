cmake_minimum_required(VERSION 3.13.1)
set(QEMU_EXTRA_FLAGS -s)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(onoff_level_lighting_vnd_app)

target_sources(app PRIVATE
	       src/main.c
	       src/app_gpio.c
	       src/storage.c
	       src/mesh/ble_mesh.c
	       src/mesh/device_composition.c
	       src/mesh/no_transition_work_handler.c
	       src/mesh/publisher.c
	       src/mesh/state_binding.c
	       src/mesh/transition.c
	       )

target_sources_ifdef(CONFIG_MCUMGR app PRIVATE src/smp_svr.c)

zephyr_include_directories(
	src/
	src/mesh
)
