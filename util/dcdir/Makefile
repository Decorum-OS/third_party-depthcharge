# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.




# List all object files here.
allobjs += command.o dcdir.o image.o region.o
allobjs += commands/print.o




.SECONDEXPANSION:

src  = $(shell pwd)
obj ?= $(src)/build

# Options to generate dependency information. Use a temporary file to make sure
# the dependency information doesn't get trashed on a failed build.
DEPFLAGS = -MT $@ -MMD -MP -MF $(obj)/$*.Td
# Move the temporary dependency information over the permanent copy.
POSTCOMPILE = mv -f $(obj)/$*.Td $(obj)/$*.d

# Force enabling c++11 features, and add this directory to the include search
# path.
CXXFLAGS := -std=c++11 -I./ $(CXXFLAGS)

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q:=@
.SILENT:
endif

all: $$(obj)/dcdir

clean:
	$(Q)rm -rf $(obj)

prefixed_objs = $(addprefix $(obj)/,$(allobjs))

$(obj)/dcdir: $$(prefixed_objs) $(MAKEFILE_LIST)
	@printf "    LD         $(subst $(obj)/,,$(@))\n"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(@) $(prefixed_objs)

$(obj)/%.o: $$(src)/$$*.cc $(MAKEFILE_LIST)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(Q)$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -o $(@) $(<)
	$(Q)$(POSTCOMPILE)

alldirs = $(obj) $(sort $(dir $(addprefix $(obj)/,$(allobjs))))
$(shell mkdir -p $(alldirs))

dependencies = $(prefixed_objs:.o=.d)
-include $(dependencies)

.PRECIOUS: $(dependencies)

.PHONY: all clean
