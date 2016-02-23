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

#include <iostream>
#include <string>

#include "command.hh"
#include "image.hh"

using ::std::cerr;
using ::std::cout;
using ::std::endl;
using ::std::ostream;
using ::std::string;

namespace {

string extract_program_name(const string &argv0);
void print_usage(ostream &out, const string &name);

} // namespace

int main(int argc, char *argv[])
{
	dcdir::Command::init_command_list();

	string prog_name = "<?>";

	if (argc > 0)
		prog_name = extract_program_name(argv[0]);

	// With no arguments, print a help message to stdout and exit nicely.
	if (argc <= 1) {
		print_usage(cout, prog_name);
		return 0;
	}

	// If help was requested, print it to stdout and exit nicely.
	string argv1 = argv[1];
	if (argc == 2 && (argv1 == "-h" || argv1 == "--help")) {
		print_usage(cout, prog_name);
		return 0;
	}

	// If help wasn't requested and there weren't enough arguments, print
	// usage informatin to stderr and return a failing exit code.
	if (argc < 3) {
		print_usage(cerr, prog_name);
		return 1;
	}

	string file_name = argv[1];
	string command_name = argv[2];

	// Read the image file into a buffer.
	dcdir::ImageBuffer buf(file_name);

	// Figure out what command the user requested.
	auto *command = dcdir::Command::find_command(command_name);
	// If no matching command was found, print help and return an error.
	if (command == NULL) {
		cerr << "Command \"" << command_name <<
			"\" not recognized." << endl << endl;
		print_usage(cerr, prog_name);
		return 1;
	}

	// Run the matching command.
	return command->run(prog_name, argc - 2, argv + 2, &buf);
}

namespace {

// Extract the filename of this command from argv0, throwing away any
// directory components of the path.
string extract_program_name(const string &argv0)
{
	auto const last_sep = argv0.find_last_of("\\/");
	if (last_sep == string::npos)
		return argv0;
	else
		return argv0.substr(last_sep + 1, argv0.length() - last_sep);
}

// Print help for the utility as a whole, and each of the commands.
void print_usage(ostream &out, const string &name)
{
	out << name << " usage:" << endl;
	out << endl;
	out << name << " [-h|--help]" << endl;
	out << name << " FILE COMMAND [PARAMETERS]" << endl;
	out << endl;
	out << "COMMANDS:" << endl;

	for (const auto &com: *dcdir::Command::command_list())
		com->usage(out);
}

}
