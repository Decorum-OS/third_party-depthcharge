from components import Area
from components import Cbfs
from components import CbfsPayload
from components import File
from components import Fmap
from components import Fwid
from components import Gbb
from components import Index
from components import Sha256
from components import Vblock
from util import MB
from util import KB

def rw_stuff():
  ecbin = File("ec.RW.bin")
  stuff = Index(
    File("depthcharge/depthcharge.payload"),
    Sha256(ecbin),
    File("coreboot_ram.stage")
  )
  return Index(ecbin), stuff

class Image(Area):
  model = "Google_Nyan"
  def __init__(self, **kwargs):
    size = 4 * MB

    fmap = Fmap(size)

    ecbina, rw_stuffa = rw_stuff()
    ecbinb, rw_stuffb = rw_stuff()

    ro_cbfs = Cbfs(
      CbfsPayload("fallback/payload",
		  File("depthcharge/depthcharge.elf")).compression("lzma")
    ).base(File("coreboot.rom"))

    gbb = Gbb(hwid="NYAN TEST 9382").expand()

    ro = fmap.section("WP_RO",
      fmap.section("RO_SECTION",
        fmap.section("COREBOOT", ro_cbfs).size(1 * MB),
        fmap.section("FMAP", fmap).size(4 * KB),
        fmap.section("GBB", gbb).expand(),
        fmap.section("RO_FRID", Fwid(self.model)).size(256).fill(0)
      ).expand(),
      fmap.section("RO_VPD").fill(0xff).size(64 * KB)
    ).size(2 * MB)

    rw_part1 = Area(
      fmap.section("RW_SECTION_A",
        fmap.section("VBLOCK_A", Vblock(rw_stuffa).expand()).size(8 * KB),
        fmap.section("FW_MAIN_A", rw_stuffa).expand(),
        Area(
          fmap.section("EC_MAIN_A", ecbina).expand(),
          fmap.section("RW_FWID_A", Fwid(self.model)).size(256).fill(0)
        ).size(128 * KB)
      ).expand(),
      fmap.section("RW_SHARED",
        fmap.section("SHARED_DATA").size(16 * KB).fill(0x00)
      ).size(16 * KB),
      fmap.section("RW_ELOG").size(16 * KB)
    ).size(512 * KB)

    rw_part2 = Area(
      fmap.section("RW_SECTION_B",
        fmap.section("VBLOCK_B", Vblock(rw_stuffb).expand()).size(8 * KB),
        fmap.section("FW_MAIN_B", rw_stuffb).expand(),
        Area(
          fmap.section("EC_MAIN_B", ecbinb).expand(),
          fmap.section("RW_FWID_B", Fwid(self.model)).size(256).fill(0),
        ).size(128 * KB)
      ).expand(),
      fmap.section("RW_VPD").size(32 * KB),
    ).size(512 * KB)

    legacy = fmap.section("RW_LEGACY").size(1 * MB)

    super(Image, self).__init__(ro, rw_part1, rw_part2, legacy)
    self.size(size)
