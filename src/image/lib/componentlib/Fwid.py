from Area import Area
from tools import ChromeosVersion

class Fwid(Area):
  _versions = None
  def __init__(self, model):
    super(Fwid, self).__init__()
    if not self._versions:
      cv = ChromeosVersion()
      self._versions = cv.get()
    self._version = model + "." + self._versions["CHROMEOS_VERSION_STRING"]
    self.size(len(self._version))

  def write(self):
    return self._version
