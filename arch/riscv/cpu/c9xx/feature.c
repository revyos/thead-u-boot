#include <common.h>
#include <asm/types.h>
#include <asm/asm.h>
#include <asm/csr.h>

void setup_features(void)
{
	unsigned int i, cpu_type, cpu_ver;
	unsigned long version[8];

	for (i = 0; i < 8; i++)
		version[i] = csr_read(CSR_MCPUID);

	cpu_type = (version[0] >> 18) & 0xf;
	cpu_ver  = (version[1] >> 12) & 0xffff;

	/*
	 * Warning: CSR_MCCR2 contains an L2 cache latency setting,
	 * you need to confirm it by your own soc design.
	 */
	switch (cpu_type) {
	case 0x3:
		if (cpu_ver >= 0x1080 && cpu_ver <= 0x10bf) { //1.2.0~1.2.x
			csr_write(CSR_MCCR2, 0xe0010009);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x6e30c);
			csr_write(CSR_MHCR, 0x1ff);
		} else if (cpu_ver == 0x10ca) { //1.3.10
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe2490009);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x66e30c);
			csr_write(CSR_MHCR, 0x17f);
			csr_write(CSR_MHINT2, 0x420000);
			csr_write(CSR_MHINT4, 0x410);
		} else if (cpu_ver >= 0x1100 && cpu_ver <= 0x113f) { //1.4.0~1.4.x
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe2490009);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x16e30c);
			csr_write(CSR_MHCR, 0x1ff);
		} else if (cpu_ver >= 0x1140 && cpu_ver <= 0x117f) { //1.5.0~1.5.x
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe2490009);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0xe6e30c);
			csr_write(CSR_MHINT2, 0x180);
			csr_write(CSR_MHCR, 0x1ff);
		} else if (cpu_ver >= 0x1180 && cpu_ver <= 0x1183) { //1.6.0~1.6.3
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe249000b);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x1ee30c);
			csr_write(CSR_MHINT2, 0x180);
			csr_write(CSR_MHCR, 0x1ff);
		} else if (cpu_ver >= 0x1184 && cpu_ver <= 0x123f) { //1.6.4~1.8.x
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe249000b);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x1ee30c);
			csr_write(CSR_MHINT2, 0x180);
			csr_write(CSR_MHCR, 0x11ff);
		} else if (cpu_ver >= 0x2000 && cpu_ver <= 0xffff) { //2.0.0~
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe249000b);
			csr_write(CSR_MXSTATUS, 0x438000);
			csr_write(CSR_MHINT, 0x31ea32c);
			csr_write(CSR_MHINT2, 0x180);
			csr_write(CSR_MHCR, 0x11ff);
		} else {
			while(1);
		}
		break;
	case 0x4:
		if (cpu_ver >= 0x1002 && cpu_ver <= 0xffff) {
			csr_write(CSR_MHCR, 0x17f);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x650c);
		} else {
			while(1);
		}
		break;
	case 0x5:
		if (cpu_ver >= 0x0000 && cpu_ver <= 0x0007) { //0.0.0~0.0.7
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xe0420008);
			csr_write(CSR_MXSTATUS, 0x638000);
			csr_write(CSR_MHINT, 0x2c50c);
			csr_write(CSR_MHCR, 0x11ff);
		} else if (cpu_ver >= 0x0040 && cpu_ver <= 0xffff) { //0.1.0~
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xa042000a);
			csr_write(CSR_MXSTATUS, 0x438000);
			csr_write(CSR_MHINT, 0x21aa10c);
			csr_write(CSR_MHCR, 0x10011ff);
		} else {
			while(1);
		}
		break;
	case 0x6:
		if (cpu_ver >= 0x0) {
			csr_write(CSR_MSMPR, 0x1);
			csr_write(CSR_MCCR2, 0xA042000A);
			csr_write(CSR_MXSTATUS, 0x638001);
			csr_write(CSR_MHINT, 0x3A1AA10C);
			csr_write(CSR_MHCR, 0x10011BF);
		} else {
			while(1);
		}
		break;
	default:
		while(1);
	}
}
