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

	detectOs
	OS=$?
	
	echo "Moving to ${CMAKE_BUILD_DIR}"
	cd "./${CMAKE_BUILD_DIR}"


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
		
		# if ! [[ "$OS" == 2 ]]; then
			
		# 	echo "Ninja config not ready for Visual Studio, Linux only, yet."
		# 	#TOOLSET="-DCMAKE_GENERATOR_TOOLSET=v142"
		# 	#cmd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
		# 	#"C:\Program Files\Git\bin\sh.exe" --login -i
		# fi
	fi	

	echo "Starting configuration with options: "
	echo "Build dir: ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
	echo "Building from: ../${CMAKE_BUILD_DIR}"
	echo "${CXX_COMPILER}"
	echo "${C_COMPILER}"
	echo "${BUILD_TYPE}"
	echo "CMAKE_J: ${CMAKE_J}"
	echo "USE_NINJA: ${USE_NINJA} ${NINJA}" 
	echo "TOOLSET: ${TOOLSET}"
	echo "OS: $OS"
	echo "${USE_CCACHE}"
	echo "${EXPORT_COMPILE_CMD}"

	X:\\cmake\\3.26.3\\win64\\bin\\cmake.exe ${NINJA} ${CONFIG_TYPE} ${BUILD_TYPE} ${CXX_COMPILER} ${C_COMPILER} ${TOOLSET} ${USE_CCACHE} ${EXPORT_COMPILE_CMD} ..

	RESULT=$?
	
	echo "Configuration done with code: ${RESULT}"

	exit $((RESULT+0))
}

build() {
	if [ -d "./${CMAKE_BUILD_DIR}" ]; then
		
		cd "${CMAKE_BUILD_DIR}"

		echo "Starting building"
		cmake --build . -j${CMAKE_J}

		RESULT=$?
		echo "Build done with code: ${RESULT}"
		
		cd "../"
		
		exit $((RESULT+0))
	else
		echo "Build dir [${CMAKE_BUILD_DIR}] does not exists"
		exit -1
	fi
}

run() {
	detectOs
	OS=$?
	PLP_BIN="PoulpeEngine"
	
	if ! [[ "$OS" == 2 ]]; then
		PLP_BIN="PoulpeEngine.exe"
	fi
	
	PLP_PATH="./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
	
	if [ -d $PLP_PATH ] ; then

		cd ${PLP_PATH}
		rm "./cache/*"
		"./${PLP_BIN}"
		RESULT=$?
		exit $((RESULT+0))		
	else
		PLP_PATH="./${CMAKE_BUILD_DIR}"
	fi

	echo $PLP_PATH
	if [ -d $PLP_PATH ] ; then
		cd ${PLP_PATH}
		rm "./cache/*"
		pwd
		"./${PLP_BIN}"
		RESULT=$?
		exit $((RESULT+0))
	else
		echo "Can't find PoulpeEngine binary"
		exit -1
	fi
	
	exit 0
}
