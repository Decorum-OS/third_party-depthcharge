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

#include "command.hh"

#include <cassert>
#include <string>
#include <vector>

#include "commands/print.hh"

using ::std::string;
using ::std::vector;

namespace dcdir
{

const vector<Command *> *Command::command_list_ = NULL;

void Command::init_command_list()
{
	assert(command_list_ == NULL);

	command_list_ = new vector<Command *>{
		new commands::Print()
	};
}

Command *Command::find_command(string name)
{
	assert(command_list_);

	for (auto &com: *command_list_)
		if (com->name() == name)
			return com;
	return NULL;
}

} // namespace dcdir
