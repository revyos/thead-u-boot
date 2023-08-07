#ifndef DDR_COMMON_FUNC_H
#define DDR_COMMON_FUNC_H

enum DDR_TYPE {
    DDR_TYPE_LPDDR4X = 0,
    DDR_TYPE_LPDDR4,
    DDR_TYPE_MAX,
};
enum DDR_BITWIDTH {
    DDR_BITWIDTH_16 = 16,
    DDR_BITWIDTH_32 = 32,
    DDR_BITWIDTH_64 = 64,
    DDR_BITWIDTH_MAX = DDR_BITWIDTH_64,
};

unsigned long get_ddr_density(void);
enum DDR_TYPE get_ddr_type(void);
void ddr_sysreg_wr(unsigned long int addr,unsigned int wr_data);
unsigned int ddr_sysreg_rd(unsigned long int addr);

void ddr_phy_reg_wr(unsigned long int addr,unsigned int wr_data);
void ddr_phy0_reg_wr(unsigned long int addr,unsigned int wr_data);
void ddr_phy1_reg_wr(unsigned long int addr,unsigned int wr_data);
unsigned int ddr_phy_reg_rd(unsigned long int addr);
unsigned int ddr_phy0_reg_rd(unsigned long int addr);
unsigned int ddr_phy1_reg_rd(unsigned long int addr);
void ddr_phy_broadcast_en(int enable);
void lp4_mrw(int addr, int wdata,int dch, int rank);
int lp4_mrr(int addr,int dch, int rank);
int lp4_set_ron(int ron);
void lp4_fast_freq_change(int target_freq);
void delay_through_apb_rd(int loop_num);
void disable_axi_port(int port);
void enable_axi_port(int port);
void quasi_reg_write(unsigned long int reg,int wdata);
void lpddr4_selfrefresh_exit(int mode);
void dfi_freq_change(int dfi_freq,int skip_dram_init);
void dfi_init(int dfi_init_complete_en);
void lpddr4_enter_selfrefresh(int pwdn_en,int dis_dram_clk,int mode);
void lpddr4_auto_ps_en(int pwdn_en,int selfref_en,int clock_auto_disable);
void ddr_soc_pll_disable(void);
void de_assert_other_reset_ddr(void);
void assert_ddrc_and_areset_sysreg(void);

void pll_config(int speed);
void deassert_pwrok_apb(enum DDR_BITWIDTH bits);
void ctrl_init(int rank_num, int speed);
void addrmap(int rank_num, enum DDR_BITWIDTH bits);
void ctrl_en(enum DDR_BITWIDTH bits);
void enable_auto_refresh(void);
void lpddr4_auto_selref(void);
#endif // DDR_COMMON_FUNCE_H
