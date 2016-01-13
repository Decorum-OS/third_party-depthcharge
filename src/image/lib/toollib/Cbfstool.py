from Tool import Tool

class Cbfstool(Tool):
  cmd = "cbfstool"

  def __init__(self, base=None, verbose=False):
    super(Cbfstool, self).__init__(verbose)
    self.base = base

  def create(self, name, size, bootblock, arch, align=None, offset=None):
    args = [self.cmd, name, "create", "-s", "%d" % size, "-B", bootblock,
	    "-m", arch]
    if align is not None:
      args.extend(["-a", "%d" % align])
    if offset is not None:
      args.extend(["-o", "%d" % offset])
    ret, stdout = self.run(args)
    if ret != 0:
      raise RuntimeError("cbfstool create failed:\n%s" % stdout)
    self.base = name

  def add(self, f, name, t, base=None):
    args = [self.cmd, self.base, "add", "-f", f, "-n", name, "-t", "%d" % t]
    if base is not None:
      args.extend(["-b", "%d"])
    ret, stdout = self.run(args)
    if ret != 0:
      raise RuntimeError("cbfstool add failed:\n%s" % stdout)

  def add_payload(self, f, name, compression=None, base=None):
    args = [self.cmd, self.base, "add-payload", "-f", f, "-n", name]
    if compression is not None:
      args.extend(["-c", compression])
    if base is not None:
      args.extend(["-b", "%d"])
    ret, stdout = self.run(args)
    if ret != 0:
      raise RuntimeError("cbfstool add-payload failed:\n%s" % stdout)
