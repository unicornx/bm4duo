#ifndef __RTC_H__
#define __RTC_H__

// RTC_CTRL_REG_BASE
#define	RTC_CTRL_VERSION	0x000
#define RTC_CTRL_UNLOCKEY	0x004
#define RTC_CTRL		0x008

#define RTC_FC_COARSE_EN	0x040
#define RTC_FC_COARSE_CAL	0x044
#define RTC_FC_FINE_EN		0x048
#define RTC_FC_FINE_PERIOD	0x04c
#define RTC_FC_FINE_CAL		0x050

// RTC_CORE_REQ_BASE
#define RTC_ANA_CALIB			0x000
#define RTC_SEC_PULSE_GEN		0x004
#define RTC_ALARM_TIME			0x008
#define RTC_ALARM_ENABLE		0x00c
#define RTC_SET_SEC_CNTR_VALUE		0x010
#define RTC_SET_SEC_CNTR_TRIG		0x014
#define RTC_SEC_CNTR_VALUE		0x018
#define RTC_INFO0			0x01c
#define RTC_INFO1			0x020
#define RTC_INFO2			0x024
#define RTC_INFO3			0x028
#define RTC_NOPOR_INFO0			0x02c
#define RTC_NOPOR_INFO1			0x030
#define RTC_NOPOR_INFO2			0x034
#define RTC_NOPOR_INFO3			0x038
#define RTC_DB_PWR_VBAT_DET		0x040
#define RTC_DB_BUTTON1			0x048
#define RTC_DB_PWR_ON			0x04c
#define RTC_7SEC_RESET			0x050
#define RTC_THM_SHDN_AUTO_REBOOT	0x064
#define RTC_POR_DB_MAGIC_KEY		0x068
#define RTC_DB_SEL_PWR			0x06c
#define RTC_UP_SEQ0			0x070
#define RTC_UP_SEQ1			0x074
#define RTC_UP_SEQ2			0x078
#define RTC_UP_SEQ3			0x07c
#define RTC_UP_IF_EN			0x080
#define RTC_UP_RSTN			0x084
#define RTC_UP_MAX			0x088
#define RTC_DN_SEQ0			0x090
#define RTC_DN_SEQ1			0x094
#define RTC_DN_SEQ2			0x098
#define RTC_DN_SEQ3			0x09c
#define RTC_DN_IF_EN			0x0a0
#define RTC_DN_RSTN			0x0a4
#define RTC_DN_MAX			0x0a8
#define RTC_PWR_CYC_MAX			0x0b0
#define RTC_WARM_RST_MAX		0x0b4
#define RTC_EN_7SEC_RST			0x0b8
#define RTC_EN_PWR_WAKEUP		0x0bc
#define RTC_EN_SHDN_REQ			0x0c0
#define RTC_EN_THM_SHDN			0x0c4
#define RTC_EN_PWR_CYC_REQ		0x0c8
#define RTC_EN_WARM_RST_REQ		0x0cc
#define RTC_EN_PWR_VBAT_DET		0x0d0
#define FSM_STATE			0x0d4
#define RTC_EN_WDG_RST_REQ		0x0e0
#define RTC_EN_SUSPEND_REQ		0x0e4
#define RTC_DB_REQ_WDG_RST		0x0e8
#define RTC_DB_REQ_SUSPEND		0x0ec
#define RTC_PG_REG			0x0f0
#define RTC_ST_ON_REASON		0x0f8
#define RTC_ST_OFF_REASON		0x0fc
#define RTC_EN_WAKEUP_REQ		0x120
#define RTC_PWR_WAKEUP_POLARITY		0x128
#define RTC_DB_SEL_REQ			0x130
#define RTC_PWR_DET_SEL			0x140

// RTC_MACRO_REQ_BASE
#define RTC_MACRO_RO_T	0x0a8

#if 0
#define RTC_ANA_CALIB 0x000
#define RTC_INFO0 0x1C
#define RTC_INFO1 0x20
#define RTC_DB_REQ_WARM_RST 0x60
#define RTC_EN_WARM_RST_REQ 0xCC
#define RTC_ST_ON_REASON 0xF8
#endif

#endif // __RTC_H__
