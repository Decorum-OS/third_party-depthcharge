from Tool import Tool

class VbutilFirmware(Tool):
  cmd = "vbutil_firmware"

  def __init__(self, verbose=False):
    super(VbutilFirmware, self).__init__(verbose)

  def vblock(self, name, keyblock, signprivate, version, fv, kernelkey,
	     flags=None):
    args = [self.cmd, "--vblock", name, "--keyblock", keyblock,
	    "--signprivate", signprivate, "--version", "%d" % version,
	    "--fv", fv, "--kernelkey", kernelkey]
    if flags is not None:
      args.extend(["--flags", "%d" % flags])
    ret, stdout = self.run(args)
    if ret != 0:
      raise RuntimeError("vbutil_firmware failed:\n%s" % stdout)
