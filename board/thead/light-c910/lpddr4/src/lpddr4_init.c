#include "../include/common_lib.h"
#include "../include/pinmux.h"
#include "../include/ddr_common_func.h"
#include "../include/ddr_retention.h"
#include "../include/lpddr4_init.h"

extern void lp4_phy_train1d2d(enum DDR_TYPE type, int speed, enum DDR_BITWIDTH bits);

void lpddr4_init(enum DDR_TYPE type, int rank_num, int speed, enum DDR_BITWIDTH bits)
{ 
  //4266 3733 3200 2133
  //Others RSVD
  pll_config(speed);
  	  
  deassert_pwrok_apb(bits);
  
  //4266 3733 3200 2133
  //Others RSVD
  ctrl_init(rank_num, speed);

  //mode support: 16 32 64
  addrmap(rank_num, bits);

  de_assert_other_reset_ddr();

  dq_pinmux(bits); // pinmux config before training

  lp4_phy_train1d2d(type, speed, bits);

  dwc_ddrphy_phyinit_regInterface(saveRegs);

  ctrl_en(bits);

  enable_axi_port(0x1f);

  enable_auto_refresh();

  lpddr4_auto_selref();
}

int fixup_ddr_addrmap(unsigned long size)
{
  enum DDR_TYPE type = get_ddr_type();
  int rank_num = get_ddr_rank_number();
  int speed = get_ddr_freq();
  enum DDR_BITWIDTH bits = get_ddr_bitwidth();

  return lpddr4_reinit_ctrl(type, rank_num, speed, bits, size);
}

int query_ddr_boundary(unsigned long size)
{
  enum DDR_TYPE type = get_ddr_type();
  int rank_num = get_ddr_rank_number();
  int speed = get_ddr_freq();
  enum DDR_BITWIDTH bits = get_ddr_bitwidth();

  return lpddr4_query_boundary(type, rank_num, speed, bits, size);
}

