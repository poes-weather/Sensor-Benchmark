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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "si21xx.h"
#include "utils.h"


//---------------------------------------------------------------------------
// Silicon Laboratories, si2107-08-09-10.pdf

#define SI21XX_BUF_SIZE     24

// allowable sample rates
#define SI2110_NUM_SAMPLE_RATES 10

static const double SAMPLE_RATES[SI2110_NUM_SAMPLE_RATES] =
{ 200, 192, 193, 194, 195, 196, 204, 205, 206, 207 };


//---------------------------------------------------------------------------
TSI21xx::TSI21xx(void)
{
    idstr = "DVB-S SI21?? Rev ?";

    i2c_rx_buf = (unsigned char*) malloc(SI21XX_BUF_SIZE);
    i2c_tx_buf = (unsigned char*) malloc(SI21XX_BUF_SIZE);

    inband_interferer_div2 = (unsigned int*) malloc(SI2110_NUM_SAMPLE_RATES * sizeof(unsigned int));
    inband_interferer_div4 = (unsigned int*) malloc(SI2110_NUM_SAMPLE_RATES * sizeof(unsigned int));

    i2c_fd = -1;

    current_fs = 0;
    current_freq = 0;
    current_symbol_rate = 0;

    lnb_lo_ = 0;
    lnb_uncertainty_ = 0;
}

//---------------------------------------------------------------------------
TSI21xx::~TSI21xx(void)
{
    i2c_close();

    free(i2c_rx_buf);
    free(i2c_tx_buf);

    free(inband_interferer_div2);
    free(inband_interferer_div4);
}

//---------------------------------------------------------------------------
bool TSI21xx::isOpen(void)
{
    return i2c_fd >= 0 ? true:false;
}

//---------------------------------------------------------------------------
bool TSI21xx::i2c_open(const char *dev)
{
    i2c_close();

    i2c_fd = ::open(dev, O_RDWR);
    if(i2c_fd < 0) {
        qDebug("Failed (%d) to open i2c: %s [%s:%d]", i2c_fd, dev, __FILE__, __LINE__);

        return false;
    }

    if(ioctl(i2c_fd, I2C_SLAVE, SI21XX_I2C_ADDR) < 0) {
        qDebug("Failed to communicate with i2c addr: 0x%04x [%s:%d]", SI21XX_I2C_ADDR, __FILE__, __LINE__);

        i2c_close();
    }

    if(!i2c_init())
        i2c_close();

    return isOpen();
}

//---------------------------------------------------------------------------
void TSI21xx::i2c_close(void)
{
    standby();

    if(isOpen())
        ::close(i2c_fd);

    i2c_fd = -1;
}

//---------------------------------------------------------------------------
void TSI21xx::standby(void)
{
    if(isOpen())
        i2c_write(sys_reg | 0x40, SI21XX_SYSMODE_REG);
}

//---------------------------------------------------------------------------
bool TSI21xx::i2c_init(void)
{
    if(!isOpen())
        return false;

    if(!i2c_read(1, SI21XX_ID_REG))
        return false;

    unsigned char byte;

    id = 2110 - (i2c_rx_buf[0] >> 4);
    rev = i2c_rx_buf[0] & 0x0f;
    idstr.sprintf("DVB-S SI%d Revision %c (%d)", id, rev + ('A' - 1), rev);
    qDebug("Id register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    if(i2c_rx_buf[0] < 0x04 || i2c_rx_buf[0] > 0x34)
        qDebug("WARNING unsupported SI21xx IC! [%s:%d]", __FILE__, __LINE__);

    i2c_read(1, SI21XX_SYSMODE_REG);
    byte = i2c_rx_buf[0];
    qDebug("System Mode register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    // make sure it is not in read only mode, PDE high
    if(byte & 0x40) {
        qDebug("NOTICE: I2C is in Sleep mode. Changing... [%s:%d]", __FILE__, __LINE__);

        byte &= ~0x40;
    }

    // make sure it is in DVB-S mode
    if((byte & 0x07) != 0x00) {
        qDebug("NOTICE: I2C is not in DVB-S mode. Changing... [%s:%d]", __FILE__, __LINE__);

        byte &= ~0x07;
    }

    // apply changes to system reg
    if(byte != i2c_rx_buf[0]) {
        i2c_write(byte, SI21XX_SYSMODE_REG);
        i2c_read(1, SI21XX_SYSMODE_REG);

        qDebug("System Mode register updated: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);
    }

    sys_reg = i2c_rx_buf[0];

    i2c_read(1, SI21XX_BYPASS_REG);
    bypass_reg = i2c_rx_buf[0];
    qDebug("Bypass register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    i2c_read(1, SI21XX_FEC_SEARCH_CTRL_1_REG);
    fec_search_param = i2c_rx_buf[0];
    qDebug("Viterbi Search Control 1 register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    i2c_read(1, SI21XX_LNB_CTRL_1_REG);
    byte = i2c_rx_buf[0];
    qDebug("LNB Control 1 register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    // disable 22kHz tone
    byte &= 0x20;
    if(i2c_rx_buf[0] != byte) {
        i2c_write(byte | 0x80, SI21XX_LNB_CTRL_1_REG);
        i2c_read(1, SI21XX_LNB_CTRL_1_REG);

        qDebug("22kHz tone set to OFF: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);
    }

    lnb_ctrl_1_reg = i2c_rx_buf[0];

    i2c_read(1, SI21XX_CARRIER_ESTIMATION_CTRL);
    cec_ctrl_reg = i2c_rx_buf[0];
    qDebug("Carrier Estimation Control register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);


    //readfrequency();

    // NOAA 2MHz 6.65400 ksps
    // Feng Yun 4 MHz 1 13.308 ksps
    //tune(1698, 6.65400 * 1.0e-3);
#if 1
    tune(1550, 22.5);
    readfrequency();
#endif

    return true;
}

//---------------------------------------------------------------------------
// The desired tuning frequency can be adjusted in intervals of 12.2 kHz
bool TSI21xx::tune(double freq_mhz, double symbol_rate_msps)
{
    qDebug("\nSet frequency: %f MHz symbolrate: %f msps [%s:%d]", freq_mhz, symbol_rate_msps, __FILE__, __LINE__);

    if(!isOpen())
        return false;
    else if(symbol_rate_msps < 0.1e-3 || symbol_rate_msps > 45)
        return false;
    else if(freq_mhz == current_freq || symbol_rate_msps == current_symbol_rate)
        return acquire();

    // all values are in MHz and MS/s
    unsigned int   coarse_tune_freq;
    unsigned short fine_tune_freq;
    unsigned char  sample_rate;
    unsigned int   inband_interferer_ind;

    double if_limit_high = -70, if_limit_low = -10;
    double dcoarse_tune_freq, dfine_tune_freq, freq;
    double band_high, band_low;
    double x1, x2, sr;
    int i;

    dcoarse_tune_freq = 10.0 * 	floor(((freq_mhz - lnb_lo_) - (if_limit_low + if_limit_high) / 2.0) / 10.0);
    dfine_tune_freq = fabs((freq_mhz - lnb_lo_) - dcoarse_tune_freq);
    freq = dcoarse_tune_freq - dfine_tune_freq;

    if(freq < 950 || freq > 2150) {
        qDebug("\nErroneous frequency: %f MHz [%s:%d]", freq, __FILE__, __LINE__);

        return false;
    }

    memset(inband_interferer_div2, 0, SI2110_NUM_SAMPLE_RATES * sizeof(unsigned int));
    memset(inband_interferer_div4, 0, SI2110_NUM_SAMPLE_RATES * sizeof(unsigned int));

    band_low = (freq_mhz - lnb_lo_) - ((lnb_uncertainty_ * 2.0) + (symbol_rate_msps * 1.35)) / 2.0;
    band_high = (freq_mhz - lnb_lo_) + ((lnb_uncertainty_ * 2.0) + (symbol_rate_msps * 1.35)) / 2.0;

    for(i=0; i<SI2110_NUM_SAMPLE_RATES; i++) {
        sr = SAMPLE_RATES[i];
        x1 = floor((freq_mhz - lnb_lo_) / (sr / 4.0)) * (sr / 4.0) + sr / 4.0;
        x2 = floor((freq_mhz - lnb_lo_) / (sr / 4.0)) * (sr / 4.0);

        if(((band_low < x1) && (x1 < band_high)) || ((band_low < x2) && (x2 < band_high)))
            inband_interferer_div4[i] = true;
    }

    for(i=0; i < SI2110_NUM_SAMPLE_RATES; i++) {
        sr = SAMPLE_RATES[i];
        x1 = floor((freq_mhz - lnb_lo_) / (sr / 2.0)) * (sr / 2.0) + sr / 2.0;
        x2 = floor((freq_mhz - lnb_lo_) / (sr / 2.0)) * (sr / 2.0);

        if(((band_low < x1) && (x1 < band_high)) || ((band_low < x2) && (x2 < band_high)))
            inband_interferer_div2[i] = true;
    }

    inband_interferer_ind = true;
    sample_rate = 0;
    for(i=0; i < SI2110_NUM_SAMPLE_RATES; i++)
        inband_interferer_ind &= inband_interferer_div2[i] | inband_interferer_div4[i];

    if(inband_interferer_ind) {
        for(i=0; i < SI2110_NUM_SAMPLE_RATES; i++) {
            if(inband_interferer_div2[i] == false) {
                sample_rate = (unsigned char) SAMPLE_RATES[i];
                break;
            }
        }
    }
    else {
        for(i=0; i < SI2110_NUM_SAMPLE_RATES; i++) {
            if((inband_interferer_div2[i] | inband_interferer_div4[i]) == false) {
                sample_rate = (unsigned char) SAMPLE_RATES[i];
                break;
            }
        }
    }

    qDebug("ADC Sampling Rate: %d MHz [%s:%d]", sample_rate,  __FILE__, __LINE__);
    if(sample_rate > 207 || sample_rate < 192) {
        sample_rate = 200;

        qDebug("WARNING: Changing ADC Sampling Rate to: %d MHz [%s:%d]", sample_rate,  __FILE__, __LINE__);
    }

    coarse_tune_freq = (unsigned char) (dcoarse_tune_freq / 10.0);
    fine_tune_freq = (unsigned short) (dfine_tune_freq / ((double) sample_rate) * 16384.0);

    qDebug("Coarse tune freq: %f MHz 0x%02x [%s:%d]", dcoarse_tune_freq, coarse_tune_freq, __FILE__, __LINE__);
    qDebug("Fine tune freq: %f MHz 0x%04x [%s:%d]", dfine_tune_freq, fine_tune_freq, __FILE__, __LINE__);
    qDebug("Tune to freq: %f MHz [%s:%d]", dcoarse_tune_freq - dfine_tune_freq, __FILE__, __LINE__);

    i = 0;
    i2c_tx_buf[i++] = SI21XX_ADC_SAMPLING_RATE;
    i2c_tx_buf[i++] = sample_rate;
    i2c_tx_buf[i++] = coarse_tune_freq;
    i2c_tx_buf[i++] = fine_tune_freq & 0xff;
    i2c_tx_buf[i++] = (fine_tune_freq >> 8) & 0xff;

    if(!i2c_write_bytes(i))
        return false;

    if(!setsymbolrate(sample_rate, symbol_rate_msps))
        return false;

    current_fs = sample_rate;
    current_symbol_rate = symbol_rate_msps;
    current_freq = freq_mhz;

    return acquire();
}

//---------------------------------------------------------------------------
bool TSI21xx::acquire(void)
{
    unsigned char acquire_reg;

    i2c_read(1, SI21XX_ACQUISITION_CTRL_1);
    acquire_reg = set_acquire_reg(i2c_rx_buf[0], false);

    if(carrier_search_range() == CESR_12_0) {
        // QuickLock
        i2c_tx_buf[0] = SI21XX_TWO_DB_THRESHOLD;
        i2c_tx_buf[1] = 0xCB;
        i2c_tx_buf[2] = 0x40;
        i2c_tx_buf[3] = 0xCB;
        i2c_write_bytes(4);

        i2c_write(0x56, SI21XX_LSA_CTRL_1);
        i2c_write(0x05, SI21XX_BLIND_SCAN_CTRL); // QuickLock mode NOTICE: bit 3 is reserved?
    }
    else {
        // alternate mode, using estimated search freq
        i2c_write(0x04, SI21XX_BLIND_SCAN_CTRL);
    }

    set_acquire_reg(acquire_reg, true);

    return true;
}

//---------------------------------------------------------------------------
unsigned char TSI21xx::set_acquire_reg(unsigned char reg, bool start)
{
    if(start)
        reg |= 0x80;
    else
        reg &= ~0x80;

    i2c_write(reg, SI21XX_ACQUISITION_CTRL_1);

    return reg;
}

//---------------------------------------------------------------------------
bool TSI21xx::setsymbolrate(double sampling_rate_mhz, double symbol_rate_msps)
{
    unsigned int sps, i;

    sps = (unsigned int) ((symbol_rate_msps * 8388608.0) / sampling_rate_mhz);

    qDebug("\nTarget sps: %d, sampling_rate_mhz: %f, symbol_rate_msps: %f [%s:%d]", sps, sampling_rate_mhz, symbol_rate_msps, __FILE__, __LINE__);

    i2c_tx_buf[0] = SI21XX_SYMBOL_RATE;
    for(i=0; i < 3; i++)
        i2c_tx_buf[i+1] = (unsigned char) ((sps >> (i * 8)) & 0xff);

    return i2c_write_bytes(4);
}

//---------------------------------------------------------------------------
double TSI21xx::readfrequency(void)
{
    if(!isOpen())
        return 0;

    double fs, sps, freq_coarse, freq_fine, freq, tmp;

    qDebug("\nRead frequency: [%s:%d]", __FILE__, __LINE__);

    // page 58, 192...207 MHz
    i2c_read(1, SI21XX_ADC_SAMPLING_RATE);
    fs = (double) i2c_rx_buf[0];
    qDebug("ADC Sampling Rate: %g MHz [%s:%d]", fs,  __FILE__, __LINE__);

    // page 66, symbol rate = reg x sampling rate / 0x00800000 Hz (>> 23)
    tmp = (double) i2c_read_3_byte(SI21XX_SYMBOL_RATE);
    sps = (tmp * fs * 1.0e6) / 8388608.0;
    qDebug("Symbol Rate: %f msps reg: %d [%s:%d]", sps * 1.0e-6, (int)tmp, __FILE__, __LINE__);

    // page 58, fc = reg x 10 MHz
    i2c_read(1, SI21XX_COARSE_TUNE_FREQ);
    freq_coarse = ((double) i2c_rx_buf[0]) * 10.0;
    qDebug("Coarse tune freq: %g MHz [%s:%d]", freq_coarse, __FILE__, __LINE__);

    // page 59, ff = reg x sampling rate / 0x00004000 (>> 14)
    tmp = (double) (i2c_read_2_byte(SI21XX_FINE_TUNE_FREQ));
    freq_fine = (tmp * fs) / 16384.0;
    qDebug("Fine tune freq: %f MHz %g [%s:%d]", freq_fine, tmp, __FILE__, __LINE__);

    freq = freq_coarse - freq_fine;

    qDebug("Current freq: %f MHz [%s:%d]", freq, __FILE__, __LINE__);

    return freq;
}

//---------------------------------------------------------------------------
bool TSI21xx::isBPSK(void)
{
    return ((sys_reg >> 3) & 0x03) == 0x00 ? true:false;
}

//---------------------------------------------------------------------------
bool TSI21xx::setdemodulation(bool bpsk)
{
    if(!isOpen())
        return false;

    unsigned char byte;

    byte = sys_reg & ~0x18; // ~0x08 should be enough...
    byte |= bpsk ? 0x00:0x08;

    if(!i2c_write(byte, SI21XX_SYSMODE_REG))
        return false;

    sys_reg = byte;

    i2c_read(1, SI21XX_SYSMODE_REG);
    qDebug("read System Mode register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    return true;
}

//---------------------------------------------------------------------------
bool TSI21xx::bypass_descrambler(void)
{
    return (bypass_reg >> 5) & 0x01 ? true:false;
}

//---------------------------------------------------------------------------
bool TSI21xx::bypass_reed_solomon(void)
{
    return (bypass_reg >> 4) & 0x01 ? true:false;
}

//---------------------------------------------------------------------------
bool TSI21xx::bypass_deinterleaver(void)
{
    return (bypass_reg >> 3) & 0x01 ? true:false;
}

//---------------------------------------------------------------------------
bool TSI21xx::bypass(bool descrambler, bool reed_solomon, bool deinterleaver)
{
    if(!isOpen())
        return false;

    unsigned char byte;

    byte = (descrambler ? 0x20:0x00) | (reed_solomon ? 0x10:0x00) | (deinterleaver ? 0x08:0);

    if(!i2c_write(byte, SI21XX_BYPASS_REG))
        return false;

    bypass_reg = byte;

    if(!i2c_read(1, SI21XX_BYPASS_REG))
        return false;

    qDebug("read Bypass register: 0x%02x [%s:%d]", i2c_rx_buf[0], __FILE__, __LINE__);

    return true;
}

//---------------------------------------------------------------------------
// notice: accept ALL, NONE or only one specific code rate
fec_t TSI21xx::fec(void)
{
    if((fec_search_param & 0x3f) == 0x3f)
        return FEC_ALL;
    else if(fec_search_param == 0x00)
        return FEC_NONE;
    else if(fec_search_param & 0x01)
        return FEC_1_2;
    else if(fec_search_param & 0x02)
        return FEC_2_3;
    else if(fec_search_param & 0x04)
        return FEC_3_4;
    else if(fec_search_param & 0x08)
        return FEC_5_6;
    else if(fec_search_param & 0x10)
        return FEC_6_7;
    else if(fec_search_param & 0x20)
        return FEC_7_8;
    else
        return FEC_ALL; // error
}

//---------------------------------------------------------------------------
bool TSI21xx::fec(fec_t new_fec)
{
    if(!isOpen())
        return false;

    unsigned char byte;

    switch(new_fec) {
    case FEC_1_2: byte = 0x01; break;
    case FEC_2_3: byte = 0x02; break;
    case FEC_3_4: byte = 0x04; break;
    case FEC_5_6: byte = 0x08; break;
    case FEC_6_7: byte = 0x10; break;
    case FEC_7_8: byte = 0x20; break;
    case FEC_ALL: byte = 0x3f; break;
    case FEC_NONE: byte = 0x00; break;
    default:
        return false;
    }

    if(i2c_write(byte, SI21XX_FEC_SEARCH_CTRL_1_REG)) {
        fec_search_param = byte;

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
carrier_search_range_t TSI21xx::carrier_search_range(void)
{
    return (carrier_search_range_t) (cec_ctrl_reg & 0x07);
}

//---------------------------------------------------------------------------
bool TSI21xx::carrier_search_range(carrier_search_range_t new_csr)
{
    if(!isOpen())
        return false;

    // page 63
    unsigned char byte = cec_ctrl_reg;

    byte &= ~0x07;
    byte |= ((unsigned char) new_csr) & 0x07;

    if(byte == cec_ctrl_reg)
        return true;

    if(i2c_write(byte, SI21XX_CARRIER_ESTIMATION_CTRL)) {
        cec_ctrl_reg = byte;

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
bool TSI21xx::isLNB_13V(void)
{
    return (lnb_ctrl_1_reg & 0x40) == 0x00 ? true:false;
}

//---------------------------------------------------------------------------
bool TSI21xx::lnb_voltage(bool volt_13)
{
    if(!isOpen())
        return false;

    unsigned char byte;

    byte = lnb_ctrl_1_reg & ~0x40;
    byte |= volt_13 ? 0x00:0x40;

    if(i2c_write(byte | 0x80, SI21XX_LNB_CTRL_1_REG)) {
        lnb_ctrl_1_reg = byte;

        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------
//
//                  SI21xx STATUS
//
//---------------------------------------------------------------------------
unsigned short TSI21xx::getlockstatus(void)
{
    if(!isOpen())
        return 0;
    else
        return i2c_read_2_byte(SI21XX_LOCK_STATUS_1);
}

//---------------------------------------------------------------------------
unsigned short TSI21xx::getacqusitionstatus(void)
{
    if(!isOpen())
        return ACQ_ACQ_FAIL;

    if(!i2c_read(1, SI21XX_ACQ_STATUS))
        return ACQ_ACQ_FAIL;

    return (unsigned short) i2c_rx_buf[0];
}

//---------------------------------------------------------------------------
// in 0 - 100%
unsigned short TSI21xx::getsignalstrength(void)
{
    if(!isOpen())
        return 0;

    unsigned int agc_th, agc_pl, strength;

    agc_th = i2c_read_byte(SI21XX_AGC_THRESHOLD);
    agc_pl = i2c_read_byte(SI21XX_AGC_POWER_LEVEL);

    strength = ((300 * agc_th * agc_pl) << 4) / 0xffff;

    return (unsigned short) (strength & 0xffff);
}

//---------------------------------------------------------------------------
// in 0 - 100%
unsigned short TSI21xx::getsnr(void)
{
    if(!isOpen())
        return 0;

    unsigned int xsnr, snr;

    xsnr = 0xffff - i2c_read_2_byte(SI21XX_AGC_CTRL_2);
    xsnr = 3 * (xsnr - 0xa100);
    snr = (xsnr > 0xffff) ? 0xffff : ((int) xsnr < 0) ? 0 : xsnr;
    snr *= (100 / 0xffff);

    return (unsigned short) (snr & 0xffff);
}

//---------------------------------------------------------------------------
// NOTE: BER must be first on, see reg 0xA0 viterbi control registers
unsigned int TSI21xx::getber(void)
{
    return i2c_read_2_byte(SI21XX_BER_COUNT);
}

//---------------------------------------------------------------------------
//
//                  I2C - RW
//
//---------------------------------------------------------------------------
unsigned int TSI21xx::i2c_read_3_byte(unsigned char regaddr_low)
{
    unsigned int rc = 0;

    if(i2c_read(3, regaddr_low))
        rc = (i2c_rx_buf[2] << 16) | (i2c_rx_buf[1] << 8) | i2c_rx_buf[0];

    return rc;
}

//---------------------------------------------------------------------------
unsigned short TSI21xx::i2c_read_2_byte(unsigned char regaddr_low)
{
    unsigned short rc = 0;

    if(i2c_read(2, regaddr_low))
        rc = (i2c_rx_buf[1] << 8) | i2c_rx_buf[0];

    return rc;
}

//---------------------------------------------------------------------------
unsigned char TSI21xx::i2c_read_byte(unsigned char regaddr)
{
    unsigned char rc = 0;

    if(i2c_read(1, regaddr))
        rc = i2c_rx_buf[0];

    return rc;
}

//---------------------------------------------------------------------------
bool TSI21xx::i2c_read(unsigned short len, unsigned char regaddr)
{
    if(!isOpen())
        return false;

    int rc;

    i2c_data.msgs = msg_i2c;
    i2c_data.nmsgs = 2;

    // write
    i2c_data.msgs[0].addr = SI21XX_I2C_ADDR;
    i2c_data.msgs[0].flags = 0;
    i2c_data.msgs[0].len = 1;
    i2c_data.msgs[0].buf = &regaddr;

    // read
    i2c_data.msgs[1].addr = SI21XX_I2C_ADDR;
    i2c_data.msgs[1].flags = I2C_M_RD;
    i2c_data.msgs[1].len = len;
    i2c_data.msgs[1].buf = i2c_rx_buf;

    rc = ioctl(i2c_fd, I2C_RDWR, &i2c_data);
    if(rc < 0) {
        qDebug("i2c read failed (%d) register: 0x%02x [%s:%d]", rc, regaddr, __FILE__, __LINE__);

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool TSI21xx::i2c_write_bytes(unsigned short len)
{
    if(!isOpen())
        return false;

    if(len < 2 /*|| (len % 2) != 0*/) {
        qDebug("i2c write failed! Erroneous length: %d [%s:%d]", len, __FILE__, __LINE__);

        return false;
    }

    i2c_data.msgs = msg_i2c;
    i2c_data.nmsgs = 1;

    i2c_data.msgs[0].addr = SI21XX_I2C_ADDR;
    i2c_data.msgs[0].flags = 0;
    i2c_data.msgs[0].len = len;
    i2c_data.msgs[0].buf = i2c_tx_buf;

    int rc = ioctl(i2c_fd, I2C_RDWR, &i2c_data);
    if(rc < 0) {
        qDebug("i2c write failed: %d [%s:%d]", rc, __FILE__, __LINE__);

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool TSI21xx::i2c_write(unsigned char byte, unsigned char regaddr)
{
    if(!isOpen())
        return false;

    i2c_tx_buf[0] = regaddr;
    i2c_tx_buf[1] = byte;

    return i2c_write_bytes(2);
}

