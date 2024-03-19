#ifndef DDR_RETENTION_H
#define DDR_RETENTION_H

///data structure to store ddr misc register address, value
typedef struct Reg_Misc_Addr_Val {
  uint32_t Address; ///< register address
  uint32_t Value;   ///< register value
} Reg_Misc_Addr_Val_t;

///data structure to store register address, value pairs
typedef struct Reg_Phy_Addr_Val {
  uint32_t Address; ///< register address
  uint16_t Value0;   ///< register value phy0
  uint16_t Value1;   ///< register value phy1
} Reg_Phy_Addr_Val_t;

/// enumeration of instructions for PhyInit Register Interface
typedef enum {
  saveRegs,         ///< save(read) tracked register values
  restoreRegs,      ///< restore (write) saved register values
} regInstr;

// typedef struct Reg_Addr_Value {
//   uint32_t reg_num;
//   Reg_Addr_Val_t reg[0];
// } Reg_Addr_Value_t;

typedef struct Ddr_Reg_Config {
    uint32_t misc_reg_num;
    uint32_t phy_reg_num;
} Ddr_Reg_Config_t;

int dwc_ddrphy_phyinit_regInterface(regInstr myRegInstr);
void dwc_ddr_misc_regu_save(void);

#endif
