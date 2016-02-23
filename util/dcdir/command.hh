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

#ifndef __COMMAND_HH__
#define __COMMAND_HH__

#include <iostream>
#include <string>
#include <vector>

#include "image.hh"

namespace dcdir
{

class Command;

using CommandList = const std::vector<Command *>;

class Command
{
public:
	static void init_command_list();

	const std::string &name() const
	{
		return name_;
	}

	// This function is called to actually do whatever the command is
	// supposed to do. prog_name is the name of the program as invoked,
	// image is the firmware image, and argc and argv are unconsumed
	// command line arguments.
	virtual int run(const std::string &prog_name, int argc, char *argv[],
			ImageBuffer *image) = 0;

	// Print help information to "out". The default implementation is
	// very basic and should be overridden.
	virtual void usage(std::ostream &out) const
	{
		out << name_ << std::endl;
	}

	// Find and return a command with the given name, or NULL if none
	// was found.
	static Command *find_command(std::string name);

	static CommandList *command_list()
	{
		return command_list_;
	}

protected:
	explicit Command(std::string new_name) : name_(new_name)
	{}

	virtual ~Command()
	{}

private:
	// The name of the command, which is how the user will select it.
	std::string name_;

	// A list of all command objects.
	static const std::vector<Command *> *command_list_;
};

} // namespace dcdir

#endif // __COMMAND_HH__
