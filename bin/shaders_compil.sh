#!/usr/bin/env bash

SECONDS=0
ROOT_DIR="$(dirname "$0")"
GLSLC_BIN="glslc"
VERBOSE=true

_print()
{
    #0 OK green
    #1 OK magenta
    #2 KO red

    text=$1
    type=$2

    if [[ $VERBOSE == true ]] ; then
        if [[ "$type" == "0" ]] ; then
            echo -e "\e[1;42m $text \e[0m"
        fi
        
        if [[ "$type" == "1" ]] ; then
            echo -e "\e[1;45m $text \e[0m"
        fi
    fi

    if [[ "$type" == "2" ]] ; then 
        echo -e "\e[1;41m $text \e[0m"
    fi
}

print_error()
{
    _print "$1" "2"
}

print_success()
{
    _print "$1" "0"
}

print_text()
{
    _print "$1" "1"
}

compile()
{
    shader=$1
    type=$2

    if  md5sum --status -c ./assets/shaders/${shader}.${type}.checksum; then
        print_text "Shader [${shader}.${type}] already up to date"
    else
        ${GLSLC_BIN} -O ./assets/shaders/${shader}.${type} -o ./assets/shaders/spv/${shader}_${type}.spv
        retval=$?
        
        if [ "$retval" != 0 ]
        then
            print_error "Compilation failed for shader [${shader}.$type]"
        else
            md5sum ./assets/shaders/${shader}.$type > ./assets/shaders/${shader}.${type}.checksum
            print_success "Shader [${shader}.$type] compiled"
        fi
    fi    
}

if [ ! -d "./assets/shaders/spv" ]; then
    mkdir "./assets/shaders/spv"
fi

print_text "Compiling shaders"

#@todo read from config/shaders.json
shaders=("main_basic" "main_basic_no_texture" "skybox"
        "shadow_map" "csm" "normal" "terrain"
        "water" "text")

for shader in ${shaders[@]}; do
    compile ${shader} "vert"
    compile ${shader} "frag"
done

geom_shaders=("normal" "terrain" "shadow_map" "csm")
for shader in ${geom_shaders[@]}; do
    compile ${shader} "geom"
done

tesc_shaders=("terrain" "water")
for shader in ${tesc_shaders[@]}; do
    compile ${shader} "tesc"
done

tese_shaders=("terrain" "water")
for shader in ${tese_shaders[@]}; do
    compile ${shader} "tese"
done

duration=$SECONDS
print_text "Done in $((duration / 60)) minutes and $((duration % 60)) seconds."
