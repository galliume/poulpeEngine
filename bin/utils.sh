#!/usr/bin/env bash

detectOs() {
	case "$(uname -sr)" in

	   Linux*Microsoft*)
		 echo 'OS detected : WSL'  # Windows Subsystem for Linux
		 return 1
		 ;;

	   Linux*)
		 echo 'OS detected : Linux'
		 return 2
		 ;;

	   CYGWIN*|MINGW*|MINGW32*|MSYS*)
		 echo 'OS detected : MS Windows'
		 return 3
		 ;;

	esac
}