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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <QSettings>
#include <QFile>
#include <QMessageBox>

#include "qboxdialog.h"
#include "ui_qboxdialog.h"
#include "utils.h"

//---------------------------------------------------------------------------

#define I2C_ADDR     0x0068 // si21xx
#define I2C_BUF_SIZE 24

//---------------------------------------------------------------------------
QboxDialog::QboxDialog(QString ini, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QboxDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    frontend_fd = -1;
    ui->tuneBtn->setEnabled(false);
    ui->frontendLabel->setText("");

    iniFile = ini;
    readSettings();

    connect(this, SIGNAL(finished(int)), this, SLOT(onQboxDialog_finished(int)));



    i2c_fd = -1;
    i2c_rx_buf = (unsigned char *) malloc(I2C_BUF_SIZE);
    i2c_tx_buf = (unsigned char *) malloc(I2C_BUF_SIZE);
}

//---------------------------------------------------------------------------
QboxDialog::~QboxDialog()
{
    delete ui;

    close_frontend();
    i2c_close();

    free(i2c_rx_buf);
    free(i2c_tx_buf);
}

//---------------------------------------------------------------------------
void QboxDialog::onQboxDialog_finished(int)
{
    close_frontend();
}

//---------------------------------------------------------------------------
bool QboxDialog::isOpen(void)
{
    return frontend_fd < 0 ? false:true;
}

//---------------------------------------------------------------------------
void QboxDialog::close_frontend(void)
{
    if(!isOpen())
        return;

    ::close(frontend_fd);
    frontend_fd = -1;

    ui->openBtn->setText("Open");
    ui->tuneBtn->setEnabled(false);
}

//---------------------------------------------------------------------------
void QboxDialog::readSettings(void)
{
    ui->signalBar->setValue(0);
    ui->snrBar->setValue(0);
    ui->berLabel->setText("0");

    QFile file(iniFile);

    if(!file.exists(iniFile))
        return;

    QSettings reg(iniFile, QSettings::IniFormat);

    reg.beginGroup("DVB");

      ui->deviceEd->setText(reg.value("Frontend", "/dev/dvb-usb/device0/frontend0").toString());
      ui->freqSpinBox->setValue(reg.value("Frequency", 1701.3).toDouble());
      ui->loSpinBox->setValue(reg.value("LO", 0).toDouble());
      ui->lnbVoltageCb->setCurrentIndex(reg.value("LNB", 0).toInt());
      ui->bandwidthSb->setValue(reg.value("Bandwidth", 4).toDouble());
      ui->symbolrateSb->setValue(reg.value("Symbolrate", 1200).toInt());
      ui->modulationCb->setCurrentIndex(reg.value("Modulation", 0).toInt());

    reg.endGroup();
}

//---------------------------------------------------------------------------
void QboxDialog::on_openBtn_clicked()
{
    if(isOpen()) {
        close_frontend();
        return;
    }


#if 1
    i2c_close();
    unsigned char tmp8[2];

    i2c_fd = i2c_open("/dev/i2c-5");
    if(i2c_fd < 0) {
        qDebug("Failed to open i2c");
        return;
    }

    // read device id from system register
    if(i2c_read(1, 0x00) > 0)
        qDebug("Device ID Register: 0x%02x, Device id: 0x%02x revision: 0x%02x",
               i2c_rx_buf[0],
               (i2c_rx_buf[0] >> 4),
               (i2c_rx_buf[0] & 0x0f));

    // read system mode register
    if(i2c_read(1, 0x01) > 0) {
        tmp8[0] = (i2c_rx_buf[0] >> 3) & 0x03;
        tmp8[1] = i2c_rx_buf[0] & 0x07;
        qDebug("System mode: 0x%02x, Modulation: %s, Mode: %s",
               i2c_rx_buf[0],
               tmp8[0] == 0x00 ? "BPSK":(tmp8[0] == 0x01 ? "QPSK":"Reservd"),
               tmp8[1] == 0x00 ? "DVB-S":(tmp8[1] == 0x01 ? "DSS":"Reservd"));
    }

    // read bypass register
    if(i2c_read(1, 0x06) > 0)
        qDebug("Bypass: 0x%02x, Descrambler:%d Reed-Solomon:%d Deinterleaver:%d",
               i2c_rx_buf[0],
               (i2c_rx_buf[0] >> 5),
               (i2c_rx_buf[0] >> 4),
               (i2c_rx_buf[0] >> 3));

    // read Viterbi Search Control 1 register
    if(i2c_read(1, 0xa0) > 0) {
        qDebug("Viterbi Search Control 1: 0x%02x, Enabled Viterbi Code Rate(s)", i2c_rx_buf[0] & 0x3f);
        qDebug("7/8 %s, 6/7 %s, 5/6 %s, 3/4 %s, 2/3 %s, 1/2 %s",
               (i2c_rx_buf[0] & 0x20 ? "YES":"NO"),
               (i2c_rx_buf[0] & 0x10 ? "YES":"NO"),
               (i2c_rx_buf[0] & 0x08 ? "YES":"NO"),
               (i2c_rx_buf[0] & 0x04 ? "YES":"NO"),
               (i2c_rx_buf[0] & 0x02 ? "YES":"NO"),
               (i2c_rx_buf[0] & 0x01 ? "YES":"NO"));
        if(i2c_rx_buf[0] & 0x3f)
            qDebug("Viterbi Code Rate is in AUTO mode");
    }

    // read Viterbi Search Status register
    if(i2c_read(1, 0xa3) > 0) {
        tmp8[0] = (i2c_rx_buf[0] >> 5);
        qDebug("Viterbi Current Code Rate register: 0x%02x", i2c_rx_buf[0]);
        if(tmp8[0] == 0x00)
            qDebug("1/2 code rate");
        else if(tmp8[0] & 0x01)
            qDebug("2/3 code rate");
        else if(tmp8[0] & 0x02)
            qDebug("3/4 code rate");
        else if(tmp8[0] & 0x03)
            qDebug("5/6 code rate");
        else if(tmp8[0] & 0x04)
            qDebug("6/7 code rate");
        else if(tmp8[0] & 0x05)
            qDebug("7/8 code rate");
        else // 11x, 0x06 or 0x07
            qDebug("undefined code rate: 0x%02x", tmp8[0]);

        qDebug("Viterbi Constellation Rotation Phase: %s", i2c_rx_buf[0] & 0x02 ? "Rotated 90":"Not rotated");
        qDebug("Viterbi I/Q Swap: %s", i2c_rx_buf[0] & 0x01 ? "Swapped":"Not swapped");
    }

    i2c_close();

#endif


    frontend_fd = ::open(ui->deviceEd->text().toStdString().c_str(), O_RDWR | O_NONBLOCK);
    if(frontend_fd < 0) {
        QMessageBox::critical(this, "Error!", "Failed to open frontend!");
        return;
    }

    ui->openBtn->setText("Close");
    ui->tuneBtn->setEnabled(true);

    readStatus();

    if(ioctl(frontend_fd, FE_GET_INFO, &fe_info) == -1) {
        QMessageBox::critical(this, "Error!", "Failed to get frontend info, FE_GET_INFO!");
        return;
    }

    ui->frontendLabel->setText(fe_info.name);

    ui->freqSpinBox->setMinimum(fe_info.frequency_min / 1000.0);
    ui->freqSpinBox->setMaximum(fe_info.frequency_max / 1000.0);
    ui->freqSpinBox->setSingleStep(fe_info.frequency_stepsize / 1000.0);

    ui->symbolrateSb->setMinimum(fe_info.symbol_rate_min / 1000);
    ui->symbolrateSb->setMaximum(fe_info.symbol_rate_max / 1000);

    ui->fetypeCb->setCurrentIndex((int) fe_info.type);

#if 1
    memset(&fe_params, 0, sizeof(struct dvb_frontend_parameters));
#else
    // si21xx doesn't have get_frontend use S2API instead
    if(ioctl(frontend_fd, FE_GET_FRONTEND, &fe_params) == -1) {
        QMessageBox::critical(this, "Error!", "Failed to get frontend, FE_GET_FRONTEND!");
        memset(&fe_params, 0, sizeof(struct dvb_frontend_parameters));
    }
#endif

    // get demodulator and tuner properties
    struct dtv_property p[11];
    struct dtv_properties props;
    props.num = 0;

    p[props.num++].cmd = DTV_FREQUENCY;
    p[props.num++].cmd = DTV_BANDWIDTH_HZ;
    p[props.num++].cmd = DTV_SYMBOL_RATE;
    p[props.num++].cmd = DTV_INNER_FEC;
    p[props.num++].cmd = DTV_VOLTAGE;
    p[props.num++].cmd = DTV_PILOT;
    p[props.num++].cmd = DTV_ROLLOFF;
    p[props.num++].cmd = DTV_DELIVERY_SYSTEM;

    props.props = p;

    if((ioctl(frontend_fd, FE_GET_PROPERTY, &props)) == -1) {
        QMessageBox::critical(this, "Error!", "Failed to get frontend properties, FE_GET_PROPERTY!");
        return;
    }

    for(unsigned i=0; i<props.num; i++)
        qDebug("props.props[%d].u.data = %d result: %d", i, (unsigned int)props.props[i].u.data, props.props[i].result);


#if 0
    switch(props.props[3].u.data) {
    case FE_QPSK:
        ui->freqSpinBox->setValue(fe_params.frequency); // it seems to be in MHz
        ui->bandwidthSb->setValue(p_status.props[1].u.data / 1000.0); // bw is in Hz
        ui->symbolrateSb->setValue(fe_params.u.qpsk.symbol_rate);
        ui->modulationCb->setCurrentIndex(0);
        ui->fecCb->setCurrentIndex((int) fe_params.u.qpsk.fec_inner);
        break;

    default:
        QMessageBox::critical(this, "Error!", "Unknown frontend type!");

    }

    // miscellaneous
    ui->systemCb->setCurrentIndex(p_status.props[2].u.data);
    ui->lnbVoltageCb->setCurrentIndex(p_status.props[3].u.data);
    ui->pilotCb->setCurrentIndex(p_status.props[4].u.data);
#endif

}

//---------------------------------------------------------------------------
void QboxDialog::readStatus(void)
{
    if(!isOpen())
        return;

    QString str;
    // fe_status_t status;
    uint16_t snr, signal;
    uint32_t ber, uncorrected_blocks;

    if(ioctl(frontend_fd, FE_READ_SIGNAL_STRENGTH, &signal) == -1)
        signal = 0;
    if(ioctl(frontend_fd, FE_READ_SNR, &snr) == -1)
        snr = 0;
    if(ioctl(frontend_fd, FE_READ_BER, &ber) == -1)
        ber = 0;
    if(ioctl(frontend_fd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks) == -1)
        uncorrected_blocks = 0;

    ui->signalBar->setValue(RINT((signal * 100) >> 16));
    str.sprintf("%.0f dBm", signal / 1000.0);
    ui->signalBar->setToolTip(str);

    ui->snrBar->setValue(RINT((snr * 100) >> 16));
    str.sprintf("%.0f dB", snr / 1000.0);
    ui->snrBar->setToolTip(str);

    str.sprintf("%d", ber);
    ui->berLabel->setText(str);

    qDebug("signal: %d", signal);
    qDebug("snr: %d", snr);
    qDebug("uncorrected blocks: %d", uncorrected_blocks);
}

//---------------------------------------------------------------------------
int QboxDialog::i2c_open(const char *dev)
{
    int fd;

    fd = ::open(dev, O_RDWR);
    if(fd < 0) {
        qDebug("Failed [%d] to open i2c: %s", fd, dev);
        return fd;
    }

    if(ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        qDebug("Failed to communicate with i2c addr: 0x%04x", I2C_ADDR);

        ::close(fd);
        fd = -1;
    }

    return fd;
}

//---------------------------------------------------------------------------
void QboxDialog::i2c_close(void)
{
    if(i2c_fd >= 0)
        ::close(i2c_fd);

    i2c_fd = -1;
}

//---------------------------------------------------------------------------
int QboxDialog::i2c_read(unsigned short len, unsigned char regaddr)
{
    if(i2c_fd < 0)
        return -1;

    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[2];
    int rc;

    i2c_data.msgs = msg;
    i2c_data.nmsgs = 2;

    // write
    i2c_data.msgs[0].addr = I2C_ADDR;
    i2c_data.msgs[0].flags = 0;
    i2c_data.msgs[0].len = 1;
    i2c_data.msgs[0].buf = &regaddr;

    // read
    i2c_data.msgs[1].addr = I2C_ADDR;
    i2c_data.msgs[1].flags = I2C_M_RD;
    i2c_data.msgs[1].len = len;
    i2c_data.msgs[1].buf = i2c_rx_buf;

    rc = ioctl(i2c_fd, I2C_RDWR, &i2c_data);
    if(rc < 0) {
        qDebug("i2c read failed: %d register: 0x%02x", rc, regaddr);
        return rc;
    }

    return len;
}
//---------------------------------------------------------------------------
