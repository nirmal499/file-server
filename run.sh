#! /usr/bin/bash

usage() { echo "Usage: $0 [-f <configure|build|run_server|run_client>] [-s <number>] [-t <number>]" 1>&2; exit 1; }

while getopts o:s:t: flag
do
    case "${flag}" in
        o) option=${OPTARG};;
        s) servert=${OPTARG};;
        t) nthread=${OPTARG};;
        *) usage;;
    esac
done

re='^[0-9]+$'
max_threads_allowed=8

# -z string: True if the string is null (an empty string)
if [ -z "${option}" ]; then
    usage
else
    if [ ${option} == "configure" ]; then
        mkdir -p build

    if [ -z "${servert}" ]; then
        usage
    fi

    if ! [[ ${servert} =~ ${re} ]] ; then
        echo "ERROR: -s flag is Not a number" >&2; exit 1
    fi

    if [ ${servert} -eq 3 ]; then
        if [ -z "${nthread}" ]; then
            usage
        fi

        if ! [[ ${nthread} =~ ${re} ]] ; then
            echo "ERROR: -t flag is Not a number" >&2; exit 1
        fi

        if [ ${nthread} -gt ${max_threads_allowed} ] ; then
            echo "ERROR: max 8 threads allowed" >&2; exit 1
        fi

        cmake -S . -B build/ -DSERVERT=${servert} -DNTHREADS=${nthread}
    else
        cmake -S . -B build/ -DSERVERT=${servert}
    fi

    # cmake -S . -B build/
    elif [ ${option} == "build" ];then
        cmake --build build/ --clean-first
    elif [ ${option} == "run_server" ];then
        cd build/driver ; ./server
    elif [ ${option} == "run_client" ];then
        cd build/driver ; ./client
    else
        echo "............Invalid data for -o flag............."
    fi
fi
