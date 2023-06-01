#!/usr/bin/env bash

configure() {
	BUILD_DIR=${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}

	if [ -d "./${BUILD_DIR}" ]; then
		echo "Directory ${BUILD_DIR} exists."

		if [ $REFRESH_BUILD_DIR = true ]; then
			rm "./${BUILD_DIR}" -rf
			echo "Directory ${BUILD_DIR} deleted."
			mkdir "./${BUILD_DIR}"
			echo "Directory ${BUILD_DIR} freshly recreated."
		fi
	else
		if [ ! -d "./${BUILD_DIR}" ]; then
			mkdir "./${BUILD_DIR}"
		fi
		
		mkdir "./${BUILD_DIR}"
		echo "Directory ${BUILD_DIR} freshly recreated."
	fi

	detectOs
	OS=$?
	
	echo "Moving to ${BUILD_DIR}"
	cd "./${BUILD_DIR}"


	NINJA=""
	CONFIG_TYPE="-DCMAKE_CONFIGURATION_TYPES=Debug;Release;RelWithDebInfo"
	BUILD_TYPE="-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
	CXX_COMPILER="-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
	C_COMPILER="-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
	TOOLSET="-DCMAKE_GENERATOR_TOOLSET=${CMAKE_TOOLSET}"
	USE_CCACHE="-DUSE_CCACHE=${USE_CCACHE}"
 	EXPORT_COMPILE_CMD="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

	if [[ "$OS" == 2 ]]; then
		TOOLSET=""
	fi
		
	if [ $USE_NINJA = true ]; then
		NINJA="-G Ninja"
		TOOLSET=""		
		
		if ! [[ "$OS" == 2 ]]; then
			
			echo "Ninja config not ready for Visual Studio, Linux only, yet."
			#TOOLSET="-DCMAKE_GENERATOR_TOOLSET=v142"
			#cmd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
			#"C:\Program Files\Git\bin\sh.exe" --login -i
		fi
	fi	

	echo "Starting configuration with options: "
	echo "Build dir: ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
	echo "Building from: ../../${CMAKE_BUILD_DIR}"
	echo "${CXX_COMPILER}"
	echo "${C_COMPILER}"
	echo "${BUILD_TYPE}"
	echo "CMAKE_J: ${CMAKE_J}"
	echo "USE_NINJA: ${USE_NINJA} ${NINJA}" 
	echo "TOOLSET: ${TOOLSET}"
	echo "OS: $OS"
	echo "${USE_CCACHE}"
	echo "${EXPORT_COMPILE_CMD}"

	cmake ${NINJA} ${CONFIG_TYPE} ${BUILD_TYPE} ${CXX_COMPILER} ${C_COMPILER} ${TOOLSET} ${USE_CCACHE} ${EXPORT_COMPILE_CMD} ../..

	RESULT=$?
	
	echo "Configuration done with code: ${RESULT}"

	#Just to be sure to be on the docking branch
	echo "Switching to ImGui docking branch"
	cd "../../vendor/imgui"
	git checkout docking
	
	exit $((RESULT+0))
}

build() {
	if [ -d "./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}" ]; then
		
		cd "${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"

		echo "Starting building into: ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
		cmake --build . -j${CMAKE_J}

		RESULT=$?
		echo "Build done with code: ${RESULT}"
		
		cd "../"
		
		exit $((RESULT+0))
	else
		echo "Build dir [${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}] does not exists"
		exit -1
	fi
}

run() {
	detectOs
	OS=$?
	RBK_BIN="Rebulkan"
	
	if ! [[ "$OS" == 2 ]]; then
		RBK_BIN="Rebulkan.exe"
	fi
	
	RBK_PATH="./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
	
	if [ -d $RBK_PATH ] ; then

		cd ${RBK_PATH}
		rm "./cache/*"
		"./${RBK_BIN}"
		RESULT=$?
		exit $((RESULT+0))		
	else
		RBK_PATH="./${CMAKE_BUILD_DIR}"
	fi

	echo $RBK_PATH
	if [ -d $RBK_PATH ] ; then
		cd ${RBK_PATH}
		rm "./cache/*"
		pwd
		"./${RBK_BIN}"
		RESULT=$?
		exit $((RESULT+0))
	else
		echo "Can't find Rebulkan binary"
		exit -1
	fi
	
	exit 0
}

runCTest() {
	cd "build/Debug"
	ctest
}