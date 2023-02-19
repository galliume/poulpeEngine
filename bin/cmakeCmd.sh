#!/usr/bin/env bash

configure() {
	if [ -d "./${CMAKE_BUILD_DIR}" ]; then
		echo "Directory ${CMAKE_BUILD_DIR} exists."

		if [ $REFRESH_BUILD_DIR = true ]; then
			rm "./${CMAKE_BUILD_DIR}" -rf
			echo "Directory ${CMAKE_BUILD_DIR} deleted."
			mkdir "./${CMAKE_BUILD_DIR}"
			echo "Directory ${CMAKE_BUILD_DIR} freshly recreated."
		fi
	else
		if [ ! -d "./${CMAKE_BUILD_DIR}" ]; then
			mkdir "./${CMAKE_BUILD_DIR}"
		fi
		
		mkdir "./${CMAKE_BUILD_DIR}"
		echo "Directory ${CMAKE_BUILD_DIR} freshly recreated."
	fi

	echo "Moving to ${CMAKE_BUILD_DIR}"
	cd "./${CMAKE_BUILD_DIR}"

	echo "Starting configuration with options : "
	echo "Build dir: ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
	echo "Building from: ../${CMAKE_BUILD_DIR}"
	echo "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}"
	echo "CMAKE_C_COMPILER : ${CMAKE_C_COMPILER}"
	echo "CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}"
	echo "CMAKE_J : ${CMAKE_J}"

	cmake -T ${CMAKE_TOOLSET} -DCMAKE_CONFIGURATION_TYPES="Debug;Release;RelWithDebInfo" -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}" ..

	echo "Configuration done"

	#Just to be sure to be on the docking branch
	echo "Switching to ImGui docking branch"
	cd "../vendor/imgui"
	git checkout docking
	
	return 0
}

build() {
	if [ -d "./${CMAKE_BUILD_DIR}" ]; then
		
		cd "${CMAKE_BUILD_DIR}"

		echo "Starting building"
		cmake --build . -j${CMAKE_J}

		RESULT=$?
		echo "Build done with code: ${RESULT}"
		
		cd "../"
		
		return 0
	else
		echo "Build dir [${CMAKE_BUILD_DIR}] does not exists"
		return -1
	fi
}

run() {
	detectOs
	OS=$?
	RBK_BIN="Rebulkan"
	
	if ! [[ "$OS" == 2 ]]; then
		RBK_BIN="Rebulkan.exe"
	fi

	sleep 2 #why some segmentation fault ? too slow to finalise building ?
	
	"./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}/${RBK_BIN}"
	
	return 0
}