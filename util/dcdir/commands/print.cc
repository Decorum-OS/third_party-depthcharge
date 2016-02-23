/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "commands/print.hh"

#include <iostream>
#include <iomanip>
#include <string>

#include "command.hh"
#include "image.hh"
#include "region.hh"

using ::std::cout;
using ::std::dec;
using ::std::endl;
using ::std::hex;
using ::std::left;
using ::std::ostream;
using ::std::right;
using ::std::setfill;
using ::std::setw;
using ::std::string;

namespace dcdir
{
namespace commands
{

Print::Print() : Command("print")
{}

namespace {

void print_region(string prefix, Region *region, bool root, bool last)
{
	cout << hex << setfill('0')
	     << "[" << setw(8) << region->offset() << " - "
	     << setw(8) << (region->offset() + region->length()) << "] "
	     << dec << setfill(' ') << prefix;

	if (!root)
		cout << (last ? "\\-" : "+-");
	cout << region->name() << endl;

	int count = region->entries().size();
	if (!root)
		prefix += (last ? "  " : "| ");
	for (int i = 0; i < count - 1; i++)
		print_region(prefix, region->entries()[i], false, false);
	if (count)
		print_region(prefix, region->entries()[count - 1], false, true);
}

} // namespace

int Print::run(const string &prog_name, int argc, char *argv[],
	       ImageBuffer *image)
{
	cout << "Found depthcharge directory structure version "
	     << image->major_version() << "." << image->minor_version()
	     << "." << endl;
	cout << endl;

	print_region("", image->root_dir(), true, true);

	cout << endl << "There are " << image->storage_overhead()
	     << " bytes of storage overhead." << endl;

	return 0;
}

void Print::usage(ostream &out) const
{
	out << setw(20) << left << name() << right
	    << "Print the structure of the dcdir." << endl;
}

} // namespace commands
} // namespace dcdir
