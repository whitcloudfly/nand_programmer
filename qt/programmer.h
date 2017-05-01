/*  Copyright (C) 2017 Bogdan Bogush <bogdan.s.bogush@gmail.com>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QTimer>
#include <cstdint>
#include <functional>
#include "serial_port_writer.h"
#include "serial_port_reader.h"

using namespace std;

enum
{
    CMD_NAND_READ_ID = 0x00,
    CMD_NAND_ERASE   = 0x01,
    CMD_NAND_READ    = 0x02,
    CMD_NAND_WRITE_S = 0x03,
    CMD_NAND_WRITE_D = 0x04,
    CMD_NAND_WRITE_E = 0x05,
    CMD_NAND_SELECT  = 0x06,
};

typedef struct __attribute__((__packed__))
{
    uint8_t code;
} Cmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t addr;
    uint32_t len;
} EraseCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t addr;
    uint32_t len;
} ReadCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t addr;
} WriteStartCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint8_t len;
    uint8_t data[];
} WriteDataCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
} WriteEndCmd;

typedef struct __attribute__((__packed__))
{
    Cmd cmd;
    uint32_t chipNum;
} SelectCmd;

enum
{
    RESP_DATA   = 0x00,
    RESP_STATUS = 0x01,
};

typedef enum
{
    STATUS_OK        = 0x00,
    STATUS_ERROR     = 0x01,
    STATUS_BAD_BLOCK = 0x02,
} StatusData;

typedef struct __attribute__((__packed__))
{
    uint8_t makerId;
    uint8_t deviceId;
    uint8_t thirdId;
    uint8_t fourthId;
} ChipId;

typedef struct __attribute__((__packed__))
{
    uint8_t code;
    uint8_t info;
    uint8_t data[];
} RespHeader;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    ChipId nandId;
} RespId;

typedef struct __attribute__((__packed__))
{
    RespHeader header;
    uint32_t addr;
} RespBadBlock;

class Programmer : public QObject
{
    Q_OBJECT

    QSerialPort serialPort;
    bool isConn;
    std::function<void(ChipId)> readChipIdCb;
    std::function<void(void)> selectChipCb;
    std::function<void(void)> eraseChipCb;
    std::function<void(int)> readChipCb;
    std::function<void(int)> writeChipCb;
    uint8_t *readChipBuf;
    uint32_t readChipLen;
    uint8_t *writeChipBuf;
    uint32_t writeChipOffset;
    uint32_t writeChipLen;
    bool isReadError;
    QTimer writeSchedTimer;

    void sendCmdCb(int status);
    int readRespHeader(const QByteArray *data, uint32_t offset,
        RespHeader *&header);
    void readRespChipIdCb(int status);
    void readRespSelectChipCb(int status);
    void readRespEraseChipCb(int status);
    void readRespReadChipCb(int status);
    void readRespWriteEndChipCb(int status);
    void sendWriteCmdCb(int status);
    void readRespWriteErrorChipCb(int status);
    void readRespWriteStartChipCb(int status);
    void sendWriteStartCmdCb(int status);
    int handleStatus(RespHeader *respHead);
    int handleWrongResp(uint8_t code);
    int handleBadBlock(QByteArray *data, uint32_t offset);
    int handleWriteError(QByteArray *data);

public:
    QByteArray readData;
    QByteArray writeData;
    SerialPortWriter *serialPortWriter;
    SerialPortReader *serialPortReader;

    explicit Programmer(QObject *parent = 0);
    ~Programmer();
    int connect();
    void disconnect();
    bool isConnected();
    void readChipId(std::function<void(ChipId)> callback);
    void eraseChip(std::function<void(void)> callback, uint32_t addr,
        uint32_t len);
    void readChip(std::function<void(int)> callback, uint8_t *buf,
        uint32_t addr, uint32_t len);
    void writeChip(std::function<void(int)> callback, uint8_t *buf,
        uint32_t addr, uint32_t len);
    void selectChip(std::function<void(void)> callback, uint32_t chipNum);

private slots:
    void sendWriteCmd();
};

#endif // PROGRAMMER_H
