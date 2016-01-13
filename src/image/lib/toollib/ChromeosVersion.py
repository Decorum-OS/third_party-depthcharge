from Tool import Tool

import re

class ChromeosVersion(Tool):
  def __init__(self, verbose=False):
    super(ChromeosVersion, self).__init__(verbose)

  def get(self):
    ret, stdout = self.run(["bash", "-c", "chromeos_version.sh"])
    if ret != 0:
      raise RuntimeError("chromeos_version.sh failed:\n%s" % stdout)

    pairs = re.findall(r"([A-Za-z_]*)=(.*)", stdout)
    version_vars = {}
    for pair in pairs:
      version_vars[pair[0]] = pair[1]
    return version_vars
