------------------------------------------------------------------------------
--  ESP - xilinx - vcu118
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

use work.grlib_config.all;
use work.amba.all;
use work.stdlib.all;
use work.devices.all;
use work.gencomp.all;
use work.memctrl.all;
use work.memoryctrl.all;
use work.leon3.all;
use work.uart.all;
use work.misc.all;
use work.net.all;
use work.jtag.all;
-- pragma translate_off
use work.sim.all;
library unisim;
use unisim.all;
-- pragma translate_on
use work.sldcommon.all;
use work.sldacc.all;
use work.tile.all;
use work.nocpackage.all;
use work.coretypes.all;
use work.config.all;
use work.socmap.all;
use work.soctiles.all;

entity top is
  generic (
    SIMULATION      : boolean := false
    );
  port (
    reset            : in    std_ulogic;
    c0_sys_clk_p     : in    std_logic;      -- 250 MHz clock
    c0_sys_clk_n     : in    std_logic;      -- 250 MHz clock
    c0_ddr4_act_n    : out   std_logic;
    c0_ddr4_adr      : out   std_logic_vector(16 downto 0);
    c0_ddr4_ba       : out   std_logic_vector(1 downto 0);
    c0_ddr4_bg       : out   std_logic_vector(0 downto 0);
    c0_ddr4_cke      : out   std_logic_vector(0 downto 0);
    c0_ddr4_odt      : out   std_logic_vector(0 downto 0);
    c0_ddr4_cs_n     : out   std_logic_vector(0 downto 0);
    c0_ddr4_ck_t     : out   std_logic_vector(0 downto 0);
    c0_ddr4_ck_c     : out   std_logic_vector(0 downto 0);
    c0_ddr4_reset_n  : out   std_logic;
    c0_ddr4_dm_dbi_n : inout std_logic_vector(7 downto 0);
    c0_ddr4_dq       : inout std_logic_vector(63 downto 0);
    c0_ddr4_dqs_c    : inout std_logic_vector(7 downto 0);
    c0_ddr4_dqs_t    : inout std_logic_vector(7 downto 0);
    --pragma translate_off
    address          : out   std_logic_vector(25 downto 0);
    data             : inout std_logic_vector(15 downto 0);
    oen              : out   std_ulogic;
    writen           : out   std_ulogic;
    romsn            : out   std_logic;
    adv              : out   std_logic;
    --pragma translate_on
    gtrefclk_p       : in    std_logic;
    gtrefclk_n       : in    std_logic;
    txp              : out   std_logic;
    txn              : out   std_logic;
    rxp              : in    std_logic;
    rxn              : in    std_logic;
    emdio            : inout std_logic;
    emdc             : out   std_ulogic;
    eint             : in    std_ulogic;
    erst             : out   std_ulogic;
    uart_rxd         : in    std_ulogic;     -- UART1_RX (u1i.rxd)
    uart_txd         : out   std_ulogic;     -- UART1_TX (u1o.txd)
    uart_ctsn        : in    std_ulogic;     -- UART1_RTSN (u1i.ctsn)
    uart_rtsn        : out   std_ulogic;     -- UART1_RTSN (u1o.rtsn)
    button           : in    std_logic_vector(3 downto 0);
    switch           : inout std_logic_vector(3 downto 0);
    led              : out   std_logic_vector(6 downto 0));
end;


architecture rtl of top is

  component sgmii_vcu118 is
    generic (
      pindex          : integer;
      paddr           : integer;
      pmask           : integer;
      abits           : integer;
      autonegotiation : integer;
      pirq            : integer;
      debugmem        : integer;
      tech            : integer;
      simulation      : boolean);
    port (
      sgmiii   : in  eth_sgmii_in_type;
      sgmiio   : out eth_sgmii_out_type;
      gmiii    : out eth_in_type;
      gmiio    : in  eth_out_type;
      reset    : in  std_logic;
      apb_clk  : in  std_logic;
      apb_rstn : in  std_logic;
      apbi     : in  apb_slv_in_type;
      apbo     : out apb_slv_out_type);
  end component sgmii_vcu118;

-- FPGA DDR4 Controller. Must be moved to FPGA partition
  component ahb2mig_up is
    generic (
      hindex : integer;
      haddr  : integer;
      hmask  : integer);
    port (
      c0_sys_clk_p     : in    std_logic;
      c0_sys_clk_n     : in    std_logic;
      c0_ddr4_act_n    : out   std_logic;
      c0_ddr4_adr      : out   std_logic_vector(16 downto 0);
      c0_ddr4_ba       : out   std_logic_vector(1 downto 0);
      c0_ddr4_bg       : out   std_logic_vector(0 downto 0);
      c0_ddr4_cke      : out   std_logic_vector(0 downto 0);
      c0_ddr4_odt      : out   std_logic_vector(0 downto 0);
      c0_ddr4_cs_n     : out   std_logic_vector(0 downto 0);
      c0_ddr4_ck_t     : out   std_logic_vector(0 downto 0);
      c0_ddr4_ck_c     : out   std_logic_vector(0 downto 0);
      c0_ddr4_reset_n  : out   std_logic;
      c0_ddr4_dm_dbi_n : inout std_logic_vector(7 downto 0);
      c0_ddr4_dq       : inout std_logic_vector(63 downto 0);
      c0_ddr4_dqs_c    : inout std_logic_vector(7 downto 0);
      c0_ddr4_dqs_t    : inout std_logic_vector(7 downto 0);
      ahbso            : out   ahb_slv_out_type;
      ahbsi            : in    ahb_slv_in_type;
      calib_done       : out   std_logic;
      rst_n_syn        : in    std_logic;
      rst_n_async      : in    std_logic;
      clk_amba         : in    std_logic;
      ui_clk           : out   std_logic;
      ui_clk_sync_rst  : out   std_logic);
  end component ahb2mig_up;

-- pragma translate_off
-- Memory model for simulation purposes only
  component ahbram_sim
    generic (
      hindex : integer := 0;
      haddr  : integer := 0;
      hmask  : integer := 16#fff#;
      tech   : integer := DEFMEMTECH;
      kbytes : integer := 1;
      pipe   : integer := 0;
      maccsz : integer := AHBDW;
      fname  : string  := "ram.dat"
      );
    port (
      rst   : in  std_ulogic;
      clk   : in  std_ulogic;
      ahbsi : in  ahb_slv_in_type;
      ahbso : out ahb_slv_out_type
      );
  end component;

-- Signals for memory controller used to boot in simulation
  signal memi : memory_in_type;
  signal memo : memory_out_type;
  signal wpo  : wprot_out_type;
  signal sdi  : sdctrl_in_type;
  signal sdo  : sdram_out_type;
-- pragma translate_on

-- Switches
  signal sel0, sel1, sel2, sel3 : std_ulogic;

-- clock and reset
  signal clkm                  : std_ulogic := '0';
  signal rstn, rstraw          : std_ulogic;
  signal lock, calib_done, rst : std_ulogic;
  signal migrstn               : std_logic;


-- Tiles
--pragma translate_off
  signal mctrl_ahbsi : ahb_slv_in_type;
  signal mctrl_ahbso : ahb_slv_out_type;
  signal mctrl_apbi  : apb_slv_in_type;
  signal mctrl_apbo  : apb_slv_out_type;
--pragma translate_on

  -- Memory controller DDR4
  signal ddr_ahbsi   : ahb_slv_in_vector_type(0 to CFG_NMEM_TILE - 1);
  signal ddr_ahbso   : ahb_slv_out_vector_type(0 to CFG_NMEM_TILE - 1);

-- DVI (unused on this board)
  signal dvi_apbi  : apb_slv_in_type;
  signal dvi_apbo  : apb_slv_out_type;
  signal dvi_ahbmi : ahb_mst_in_type;
  signal dvi_ahbmo : ahb_mst_out_type;

-- Ethernet
  signal gmiii            : eth_in_type;
  signal gmiio            : eth_out_type;
  signal sgmiii           : eth_sgmii_in_type;
  signal sgmiio           : eth_sgmii_out_type;
  signal sgmiirst         : std_logic;
  signal ethernet_phy_int : std_logic;
  signal rxd1             : std_logic;
  signal txd1             : std_logic;
  signal ethi             : eth_in_type;
  signal etho             : eth_out_type;
  signal egtx_clk         : std_ulogic;
  signal negtx_clk        : std_ulogic;
  constant CPU_FREQ       : integer := 156250;  -- cpu frequency in KHz
  signal eth0_apbi        : apb_slv_in_type;
  signal eth0_apbo        : apb_slv_out_type;
  signal sgmii0_apbi      : apb_slv_in_type;
  signal sgmii0_apbo      : apb_slv_out_type;
  signal eth0_ahbmi       : ahb_mst_in_type;
  signal eth0_ahbmo       : ahb_mst_out_type;
  signal edcl_ahbmo : ahb_mst_out_type;

-- DSU
  signal ndsuact : std_ulogic;
  signal dsuerr  : std_ulogic;

-- NOC
  signal chip_rst       : std_ulogic;
  signal noc_clk        : std_ulogic;
  signal chip_refclk    : std_ulogic;
  signal chip_pllbypass : std_logic_vector(CFG_TILES_NUM-1 downto 0);
  signal chip_pllclk    : std_ulogic;

  attribute keep         : boolean;
  attribute syn_keep     : string;
  attribute keep of clkm : signal is true;

begin

-------------------------------------------------------------------------------
-- Leds -----------------------------------------------------------------------
-------------------------------------------------------------------------------

  -- From DSU 0 (on chip)
  led0_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(0), ndsuact);
  -- From CPU 0 (on chip)
  led1_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(1), dsuerr);
  --pragma translate_off
  process(clkm, rstn)
  begin  -- process
    if rstn = '1' then
      assert dsuerr = '0' report "Program Completed!" severity failure;
    end if;
  end process;
  --pragma translate_on

  -- From DDR controller (on FPGA)
  led2_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(2), calib_done);
  led3_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(3), lock);
  led4_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(4), ddr_ahbso(0).hready);

  -- unused
  led5_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(5), '0');
  led6_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (led(6), '0');

-------------------------------------------------------------------------------
-- Switches -------------------------------------------------------------------
-------------------------------------------------------------------------------

  sw0_pad : iopad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (switch(0), '0', '1', sel0);
  sw1_pad : iopad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (switch(1), '0', '1', sel1);
  sw2_pad : iopad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (switch(2), '0', '1', sel2);
  sw3_pad : iopad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v, strength => 8)
    port map (switch(3), '0', '1', sel3);

-------------------------------------------------------------------------------
-- Buttons --------------------------------------------------------------------
-------------------------------------------------------------------------------

  --pio_pad : inpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
  --  port map (button(i-4), gpioi.din(i));

----------------------------------------------------------------------
--- FPGA Reset and Clock generation  ---------------------------------
----------------------------------------------------------------------

  reset_pad : inpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x12v) port map (reset, rst);
  rst0      : rstgen                    -- reset generator
    generic map (acthigh => 1, syncin => 0)
    port map (rst, clkm, lock, rstn, rstraw);
  lock <= calib_done;

  rst1 : rstgen                         -- reset generator
    generic map (acthigh => 1)
    port map (rst, clkm, lock, migrstn, open);



  -- pragma translate_off
----------------------------------------------------------------------
---  Memory controllers ----------------------------------------------
----------------------------------------------------------------------
  -- Memory controller is required for current testbench, because it drives a
  -- boot ROM. On the final system, instead, there is no ROM and the system
  -- boots from DRAM thanks to grmon and the DSU.
  memi.writen <= '1'; memi.wrn <= "1111"; memi.bwidth <= "01";
  memi.brdyn  <= '0'; memi.bexcn <= '1';

  mctrl0 : mctrl generic map (hindex    => 0, pindex => 0,
                              paddr     => 0, srbanks => 2, ram8 => 1,
                              ram16     => 1, sden => CFG_MCTRL_SDEN,
                              invclk    => 0, sepbus => CFG_MCTRL_SEPBUS,
                              pageburst => CFG_MCTRL_PAGE, rammask => 0, iomask => 0)
    port map (rstn, clkm, memi, memo, mctrl_ahbsi, mctrl_ahbso, mctrl_apbi, mctrl_apbo, wpo, sdo);

  addr_pad : outpadv generic map (width => 26, tech => CFG_PADTECH, level => cmos, voltage => x18v)
    port map (address(25 downto 0), memo.address(26 downto 1));
  roms_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
    port map (romsn, memo.romsn(0));
  oen_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
    port map (oen, memo.oen);
  adv_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
    port map (adv, '0');
  wri_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
    port map (writen, memo.writen);
  data_pad : iopadvv generic map (tech => CFG_PADTECH, width => 16, level => cmos, voltage => x18v)
    port map (data(15 downto 0), memo.data(31 downto 16),
              memo.vbdrive(31 downto 16), memi.data(31 downto 16));
  -- pragma translate_on



----------------------------------------------------------------------
---  DDR4 memory controller ------------------------------------------
----------------------------------------------------------------------

  gen_mig : if (SIMULATION /= true) generate
    ddrc : ahb2mig_up
      generic map (
        hindex => 4,
        haddr  => 16#400#,
        hmask  => 16#C00#)
      port map (
        c0_sys_clk_p     => c0_sys_clk_p,
        c0_sys_clk_n     => c0_sys_clk_n,
        c0_ddr4_act_n    => c0_ddr4_act_n,
        c0_ddr4_adr      => c0_ddr4_adr,
        c0_ddr4_ba       => c0_ddr4_ba,
        c0_ddr4_bg       => c0_ddr4_bg,
        c0_ddr4_cke      => c0_ddr4_cke,
        c0_ddr4_odt      => c0_ddr4_odt,
        c0_ddr4_cs_n     => c0_ddr4_cs_n,
        c0_ddr4_ck_t     => c0_ddr4_ck_t,
        c0_ddr4_ck_c     => c0_ddr4_ck_c,
        c0_ddr4_reset_n  => c0_ddr4_reset_n,
        c0_ddr4_dm_dbi_n => c0_ddr4_dm_dbi_n,
        c0_ddr4_dq       => c0_ddr4_dq,
        c0_ddr4_dqs_c    => c0_ddr4_dqs_c,
        c0_ddr4_dqs_t    => c0_ddr4_dqs_t,
        ahbso            => ddr_ahbso(0),
        ahbsi            => ddr_ahbsi(0),
        calib_done       => calib_done,
        rst_n_syn        => migrstn,
        rst_n_async      => rstraw,
        clk_amba         => clkm,
        ui_clk           => clkm,
        ui_clk_sync_rst  => open
        );

  end generate gen_mig;

  gen_mig_model : if (SIMULATION = true) generate
    -- pragma translate_off

    mig_ahbram : ahbram_sim
      generic map (
        hindex => 4,
        haddr  => 16#400#,
        hmask  => 16#C00#,
        tech   => 0,
        kbytes => 1000,
        pipe   => 0,
        maccsz => AHBDW,
        fname  => "ram.srec"
        )
      port map(
        rst   => rstn,
        clk   => clkm,
        ahbsi => ddr_ahbsi(0),
        ahbso => ddr_ahbso(0)
        );

    c0_ddr4_act_n    <= '1';
    c0_ddr4_adr      <= (others => '0');
    c0_ddr4_ba       <= (others => '0');
    c0_ddr4_bg       <= (others => '0');
    c0_ddr4_cke      <= (others => '0');
    c0_ddr4_odt      <= (others => '0');
    c0_ddr4_cs_n     <= (others => '0');
    c0_ddr4_ck_t     <= (others => '0');
    c0_ddr4_ck_c     <= (others => '0');
    c0_ddr4_reset_n  <= '1';
    c0_ddr4_dm_dbi_n <= (others => 'Z');
    c0_ddr4_dq       <= (others => 'Z');
    c0_ddr4_dqs_c    <= (others => 'Z');
    c0_ddr4_dqs_t    <= (others => 'Z');

    --calib_done        : out   std_logic;
    calib_done <= '1';
    --ui_clk            : out   std_logic;
    clkm       <= not clkm after 1.667 ns;
  --ui_clk_sync_rst   : out   std_logic
  -- n/a
  -- pragma translate_on
  end generate gen_mig_model;

-----------------------------------------------------------------------
---  ETHERNET ---------------------------------------------------------
-----------------------------------------------------------------------

  eth0 : if SIMULATION = false and CFG_GRETH = 1 generate      -- Gaisler ethernet MAC
    e1 : grethm
      generic map(
        hindex       => CFG_AHB_JTAG,
        ehindex      => CFG_AHB_JTAG + 1,
        pindex       => 14,
        paddr        => 16#800#,
        pmask        => 16#f00#,
        pirq         => 12,
        memtech      => CFG_MEMTECH,
        mdcscaler    => CPU_FREQ/1000,
        rmii         => 0,
        enable_mdio  => 1,
        fifosize     => CFG_ETH_FIFO,
        nsync        => 2,
        edcl         => CFG_DSU_ETH,
        edclbufsz    => CFG_ETH_BUF,
        phyrstadr    => 3,
        vcu118       => 1,
        macaddrh     => CFG_ETH_ENM,
        macaddrl     => CFG_ETH_ENL,
        enable_mdint => 1,
        ipaddrh      => CFG_ETH_IPM,
        ipaddrl      => CFG_ETH_IPL,
        giga         => CFG_GRETH1G,
        ramdebug     => 0,
        gmiimode     => 1,
        edclsepahbg => 1)
      port map(
        rst   => rstn,
        clk   => clkm,
        ahbmi => eth0_ahbmi,
        ahbmo => eth0_ahbmo,
        apbi  => eth0_apbi,
        eahbmo => edcl_ahbmo,
        apbo  => eth0_apbo,
        ethi  => gmiii,
        etho  => gmiio);

    sgmiirst <= not rstraw;

    sgmii0 : sgmii_vcu118
      generic map(
        pindex          => 15,
        paddr           => 16#010#,
        pmask           => 16#ff0#,
        abits           => 8,
        autonegotiation => 1,
        pirq            => 11,
        debugmem        => 1,
        tech            => CFG_FABTECH,
        simulation      => SIMULATION
        )
      port map(
        sgmiii   => sgmiii,
        sgmiio   => sgmiio,
        gmiii    => gmiii,
        gmiio    => gmiio,
        reset    => sgmiirst,
        apb_clk  => clkm,
        apb_rstn => rstn,
        apbi     => sgmii0_apbi,
        apbo     => sgmii0_apbo
        );

    emdio_pad : iopad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
      port map (emdio, sgmiio.mdio_o, sgmiio.mdio_oe, sgmiii.mdio_i);

    emdc_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
      port map (emdc, sgmiio.mdc);

    eint_pad : inpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
      port map (eint, sgmiii.mdint);

    erst_pad : outpad generic map (tech => CFG_PADTECH, level => cmos, voltage => x18v)
      port map (erst, sgmiio.reset);

    sgmiii.clkp <= gtrefclk_p;
    sgmiii.clkn <= gtrefclk_n;
    txp         <= sgmiio.txp;
    txn         <= sgmiio.txn;
    sgmiii.rxp  <= rxp;
    sgmiii.rxn  <= rxn;

  end generate;

  no_eth0 : if SIMULATION = true or CFG_GRETH = 0 generate
    eth0_apbo   <= apb_none;
    sgmii0_apbo <= apb_none;
    eth0_ahbmo  <= ahbm_none;
    edcl_ahbmo <= ahbm_none;
    txp         <= '0';
    txn         <= '1';
    emdc        <= '0';
    erst        <= '0';
    emdio       <= '0';
  end generate;

  -----------------------------------------------------------------------------
  -- CHIP
  -----------------------------------------------------------------------------
  chip_rst       <= rstn;
  noc_clk        <= clkm;
  chip_refclk    <= clkm;
  chip_pllbypass <= (others => '0');

  esp_1 : esp
    port map (
      rst                => chip_rst,
      noc_clk            => noc_clk,
      refclk             => chip_refclk,
      mem_clk            => noc_clk,
      pllbypass          => chip_pllbypass,
      --pragma translate_off
      mctrl_ahbsi        => mctrl_ahbsi,
      mctrl_ahbso        => mctrl_ahbso,
      mctrl_apbi         => mctrl_apbi,
      mctrl_apbo         => mctrl_apbo,
      --pragma translate_on
      uart_rxd           => uart_rxd,
      uart_txd           => uart_txd,
      uart_ctsn          => uart_ctsn,
      uart_rtsn          => uart_rtsn,
      ndsuact            => ndsuact,
      dsuerr             => dsuerr,
      ddr_ahbsi          => ddr_ahbsi,
      ddr_ahbso          => ddr_ahbso,
      eth0_ahbmi         => eth0_ahbmi,
      eth0_ahbmo         => eth0_ahbmo,
      edcl_ahbmo         => edcl_ahbmo,
      eth0_apbi          => eth0_apbi,
      eth0_apbo          => eth0_apbo,
      sgmii0_apbi        => sgmii0_apbi,
      sgmii0_apbo        => sgmii0_apbo,
      dvi_apbi           => dvi_apbi,
      dvi_apbo           => dvi_apbo,
      dvi_ahbmi          => dvi_ahbmi,
      dvi_ahbmo          => dvi_ahbmo);

end;