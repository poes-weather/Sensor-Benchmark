/*
    POES-Weather Ltd Sensor Benchmark, a software for testing different sensors used to align antennas.
    Copyright (C) 2011 Free Software Foundation, Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: <postmaster@poes-weather.com>
    Web: <http://www.poes-weather.com>
*/

//---------------------------------------------------------------------------
#ifndef SI21XX_H
#define SI21XX_H

#include <QString>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

//---------------------------------------------------------------------------
typedef enum
{
    FEC_1_2 = 0,
    FEC_2_3,
    FEC_3_4,
    FEC_5_6,
    FEC_6_7,
    FEC_7_8,
    FEC_ALL,
    FEC_NONE
} fec_t;

typedef enum
{
    CESR_12_0 = 0,  // +- in Mhz, +- fs/16 (QuickLock)
    CESR_6_3,       // default
    CESR_3_1,
    CESR_1_6,
    CESR_0_8,
    CESR_0_4,
    CESR_0_2,
    CESR_0_1        // +- fs/2048
} carrier_search_range_t;

//---------------------------------------------------------------------------
//                  I2C REGISTERS
//---------------------------------------------------------------------------
#define SI21XX_I2C_ADDR                 0x0068 // pulldown

#define SI21XX_ID_REG                   0x00
#define SI21XX_SYSMODE_REG              0x01
#define SI21XX_BYPASS_REG               0x06
#define SI21XX_FEC_SEARCH_CTRL_1_REG    0xA0
#define SI21XX_LNB_CTRL_1_REG           0xC0

#define SI21XX_ACQUISITION_CTRL_1       0x14 // page 57
#define SI21XX_ADC_SAMPLING_RATE        0x15 // page 58, fs = 192...207 MHz, x 1 MHz
#define SI21XX_COARSE_TUNE_FREQ         0x16 // page 58, fc = reg x 10 MHz
#define SI21XX_FINE_TUNE_FREQ           0x17 // page 59, fi = reg x sampling rate / 0x00004000 (>> 14)
#define SI21XX_SYMBOL_RATE              0x3F // page 66, symbol rate = reg x sampling rate / 0x00800000 Hz (>> 23)
#define SI21XX_AGC_CTRL_1               0x23 // page 60
#define SI21XX_AGC_CTRL_2               0x24 // page 61
#define SI21XX_AGC_1_2_GAIN             0x25
#define SI21XX_AGC_3_4_GAIN             0x26
#define SI21XX_AGC_THRESHOLD            0x27 // page 62
#define SI21XX_AGC_POWER_LEVEL          0x28 // page 62
#define SI21XX_CARRIER_ESTIMATION_CTRL  0X29 // page 62
#define SI21XX_BLIND_SCAN_CTRL          0x80 // page 72
#define SI21XX_LSA_CTRL_1               0x8d // page 75
#define SI21XX_TWO_DB_THRESHOLD         0x91 // page 76
#define SI21XX_BER_COUNT                0xAB // page 79

#define	SI21XX_LOCK_STATUS_1            0x0f // page 55
#define	SI21XX_ACQ_STATUS               0x11 // page 57

//---------------------------------------------------------------------------
//                  LOCK STATUS
//---------------------------------------------------------------------------
#define	LOCK_AGC_DONE       0x4000  // AGC Lock Complete
#define	LOCK_CE_DONE        0x2000  // Carrier Estimation Complete
#define	LOCK_SR_DONE        0x1000  // Symbol Rate Estimation Complete
#define	LOCK_ST_LOCK        0x0800  // Symbol Timing Lock
#define	LOCK_CR_LOCK        0x0400  // Carrier Lock
#define	LOCK_VT_LOCK        0x0200  // Viterbi Lock
#define	LOCK_FS_LOCK        0x0100  // Frame Sync Lock
#define	LOCK_RCV_LOCK       0x0080  // Receiver Lock
#define	LOCK_BSD_READY      0x0002  // Blindscan Data Ready for host to read
#define	LOCK_BS_DONE        0x0001  // Blindscan Done

//---------------------------------------------------------------------------
//                  ACQUSITION STATUS
//---------------------------------------------------------------------------
#define	ACQ_ACQ_FAIL        0x0080  // Acquisition failed
#define	ACQ_AGC_FAIL        0x0040  // AGC Search failed
#define	ACQ_CE_FAIL         0x0020  // Carrier Estimation Search failed
#define	ACQ_SR_FAIL         0x0010  // Symbol Rate Search failed
#define	ACQ_ST_FAIL         0x0008  // Symbol Timing Search failed
#define	ACQ_CR_FAIL         0x0004  // Carrier Search failed
#define	ACQ_VT_FAIL         0x0002  // Viterbi Search failed
#define	ACQ_FS_FAIL         0x0001  // Frame Sync Search failed


//---------------------------------------------------------------------------
class TSI21xx
{
public:
    TSI21xx(void);
    ~TSI21xx(void);

    bool isOpen(void);
    bool i2c_open(const char *dev);
    void i2c_close(void);
    void standby(void);

    unsigned int   i2c_read_3_byte(unsigned char regaddr_low);
    unsigned short i2c_read_2_byte(unsigned char regaddr_low);
    unsigned char  i2c_read_byte(unsigned char regaddr);

    bool i2c_write(unsigned char byte, unsigned char regaddr);
    bool i2c_write_bytes(unsigned short len);

    QString getIdStr(void) const { return idstr; }
    int     getId(void) const { return id; }

    bool isBPSK(void);
    bool setdemodulation(bool bpsk = true);

    bool bypass_descrambler(void);
    bool bypass_reed_solomon(void);
    bool bypass_deinterleaver(void);
    bool bypass(bool descrambler, bool reed_solomon, bool deinterleaver);

    fec_t fec(void);
    bool  fec(fec_t new_fec);

    bool   isLNB_13V(void);
    bool   lnb_voltage(bool volt_13 = true);
    double lnb_lo(void) const    { return lnb_lo_; }
    void   lnb_lo(double lo_mhz) { lnb_lo_ = lo_mhz; }

    carrier_search_range_t carrier_search_range(void);
    bool carrier_search_range(carrier_search_range_t new_csr);

    bool tune(double freq_mhz, double symbol_rate_sps);

    unsigned short getlockstatus(void);
    unsigned short getacqusitionstatus(void);
    unsigned int   getber(void);
    unsigned short getsnr(void);
    unsigned short getsignalstrength(void);


protected:
    bool i2c_init(void);
    bool i2c_read(unsigned short len, unsigned char regaddr);

    double readfrequency(void);
    bool   setsymbolrate(double sampling_rate_mhz, double symbol_rate_msps);
    bool   acquire(void);
    unsigned char set_acquire_reg(unsigned char reg, bool start);

private:
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg_i2c[2];

    int i2c_fd;
    unsigned char *i2c_rx_buf, *i2c_tx_buf;
    unsigned char sys_reg, bypass_reg, fec_search_param;
    unsigned char cec_ctrl_reg;

    unsigned char lnb_ctrl_1_reg;
    double        lnb_lo_, lnb_uncertainty_; // MHz

    double        current_freq, current_symbol_rate, current_fs; // mhz & msps

    QString idstr;
    int id, rev;

    unsigned int *inband_interferer_div2, *inband_interferer_div4;
};

#endif // SI21XX_H
