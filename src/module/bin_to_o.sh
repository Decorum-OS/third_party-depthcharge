#!/bin/bash

##
## Copyright 2016 Google Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

# Use incremental linking to wrap a binary file into a .o.

set -e -u

OUTPUT="${1}"
# The first option is the file name to put the wrapped binary file into.

NAME="${2}"
# The second option is the name the binary will have when it's been wrapped.
# gcc/ld will create symbols called _binary_${NAME}_start,
# _binary_${NAME}_end, and _binary_${NAME}_size. The _start and _end symbols
# are relative and relocated when the binary data is moved during later
# linking.

BINARY="${3}"
# The path to the binary file to wrap.

shift 3
SYMBOLS=("$@")
# The remaining arguments define additional symbols to inject into the .o.
# file. These symbols will all be absolute.

if [[ -z "${OUTPUT}" ]]; then
	>&2 echo "No output file specified."
	exit 1
fi

if [[ -z "${NAME}" ]]; then
	>&2 echo "No name specified."
	exit 2
fi

if [[ -z "${BINARY}" ]]; then
	>&2 echo "No binary file specified."
	exit 3
fi

TEMPDIR=$(mktemp -d -t "${NAME}.XXXXXXXX")
function clean_up_temp_dir {
	rm -rf "${TEMPDIR}"
}
trap clean_up_temp_dir EXIT

cp "${BINARY}" "${TEMPDIR}/${NAME}"

SYMBOL_OPTS=""
if [[ ${#SYMBOLS[@]} -ne 0 ]]; then
	SYMBOL_OPTS="${SYMBOLS[@]/#/-Wl,--defsym=}"
fi

pushd "${TEMPDIR}" > /dev/null
"${CC}" -Wl,--relocatable -Wl,-b,binary -fuse-ld=bfd -nostdlib \
	-o "${NAME}".bin.o -Xlinker "${NAME}" ${SYMBOL_OPTS}
	
popd > /dev/null

cp "${TEMPDIR}/${NAME}.bin.o" "${OUTPUT}"
