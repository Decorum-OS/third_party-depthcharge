from Tool import Tool

class GbbUtility(Tool):
  cmd = "gbb_utility"

  def __init__(self, base=None, verbose=False):
    super(GbbUtility, self).__init__(verbose)
    self.base = None

  def create(self, sizes, name):
    sizestr = ",".join(["%#X" % size for size in sizes])
    ret, stdout = self.run([self.cmd, "-c", sizestr, name])
    if ret != 0:
      raise RuntimeError("gbb_utility create failed:\n%s" % stdout)
    self.base = name

  def set(self, hwid=None, flags=None, bmpfv=None,
	  rootkey=None, recoverykey=None):
    args = [self.cmd, "--set"]
    if hwid is not None:
      args.append("--hwid=%s" % hwid)
    if flags is not None:
      args.append("--flags=%d" % flags)
    if bmpfv is not None:
      args.append("--bmpfv=%s" % bmpfv)
    if rootkey is not None:
      args.append("--rootkey=%s" % rootkey)
    if recoverykey is not None:
      args.append("--recoverykey=%s" % recoverykey)
    args.append(self.base)
    ret, stdout = self.run(args)
    if ret != 0:
      raise RuntimeError("gbb_utility set failed:\n%s" % stdout)
