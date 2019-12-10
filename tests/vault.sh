#!/bin/bash

set -e
################################################################
#                   Replace the variables                      #
################################################################
NAME=timing_tests
VAULT_DIR=$(cd `dirname $0` && pwd)
BUILD_COMMAND=make
OUTPUT_DIR=$VAULT_DIR/../../buildroot_overlay/root/$NAME
EYRIE_DIR=$KEYSTONE_SDK_DIR/rts/eyrie
EYRIE_PLUGINS="freemem env_setup linux_syscall untrusted_io_syscall"
PACKAGE_FILES="untrusted/untrusted.eapp_riscv \
               malloc_timing/malloc_timing.eapp_riscv \
               test-runner.riscv \
               test \
               $EYRIE_DIR/eyrie-rt"
PACKAGE_SCRIPT="./test"

################################################################
#                       Sanity Check                           #
################################################################

# check if KEYSTONE_SDK_DIR is set
if [[ $KEYSTONE_SDK_DIR = "" ]]; then
  echo "KEYSTONE_SDK_DIR is not set"
  exit 1
fi

if [[ ! -d $KEYSTONE_SDK_DIR ]]; then
  echo "Invalid KEYSTONE_SDK_DIR '$KEYSTONE_SDK_DIR'"
  exit 1
fi

# check if riscv tools are in PATH
if ! (
  $(command -v riscv64-unknown-elf-g++ > /dev/null) &&
  $(command -v riscv64-unknown-linux-gnu-g++ > /dev/null) &&
  $(command -v riscv64-unknown-elf-gcc > /dev/null) &&
  $(command -v riscv64-unknown-linux-gnu-gcc > /dev/null)
  ); then
  echo "riscv tools are not in PATH"
  exit 1
fi

# check if OUTPUT_DIR is set
if [[ $OUTPUT_DIR = "" ]]; then
  echo "OUTPUT_DIR is not set"
  exit 1
fi

# check if EYRIE_DIR is valid
if [[ ! -d $EYRIE_DIR ]]; then
  echo "Invalid EYRIE_DIR '$EYRIE_DIR'"
  exit 1
fi

################################################################
#                       Build Enclave                          #
################################################################

# create a build directory
OUTPUT_FILES_DIR=$OUTPUT_DIR/files
mkdir -p $OUTPUT_FILES_DIR

# build eyrie runtime
$EYRIE_DIR/build.sh $EYRIE_PLUGINS

# build the app
pushd $VAULT_DIR
$BUILD_COMMAND
for output in $PACKAGE_FILES; do
  cp $output $OUTPUT_FILES_DIR
done
popd

# create vault archive & remove output files
makeself --noprogress "$OUTPUT_FILES_DIR" "$OUTPUT_DIR/$NAME.ke" "Keystone vault archive" "$PACKAGE_SCRIPT"
rm -rf $OUTPUT_FILES_DIR
