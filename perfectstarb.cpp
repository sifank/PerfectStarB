/*******************************************************************************
  Copyright(c) 2015 Jasem Mutlaq. All rights reserved.
  Modified for Wa-chur-ed Observatory's PerfectStarB by Sifan Kahale 2022

 PerfectStarB Focuser

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.
 .
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.
 .
 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#include "perfectstarb.h"

#include <cmath>
#include <cstring>
#include <memory>

#define PERFECTSTAR_TIMEOUT 1000 /* 1000 ms */

#define FOCUS_SETTINGS_TAB "Settings"

// We declare an auto pointer to PerfectStarB.
static std::unique_ptr<PerfectStarB> perfectStar(new PerfectStarB());

PerfectStarB::PerfectStarB()
{
    FI::SetCapability(FOCUSER_CAN_ABS_MOVE | FOCUSER_CAN_REL_MOVE | FOCUSER_CAN_ABORT | FOCUSER_CAN_SYNC);
    setSupportedConnections(CONNECTION_NONE);
}

bool PerfectStarB::Connect()
{
    sim = isSimulation();

    if (sim)
    {
        SetTimer(getCurrentPollingPeriod());
        return true;
    }

    handle = hid_open(0x04D8, 0xEFC4, nullptr);

    if (handle == nullptr)
    {
        LOG_ERROR("No PerfectStarB focuser found.");
        return false;
    }
    else
        SetTimer(getCurrentPollingPeriod());

    return (handle != nullptr);
}

bool PerfectStarB::Disconnect()
{
    if (!sim)
    {
        hid_close(handle);
        hid_exit();
    }

    return true;
}

const char *PerfectStarB::getDefaultName()
{
    return (const char *)"PerfectStarB";
}

bool PerfectStarB::initProperties()
{
    INDI::Focuser::initProperties();

    // Max Position
//    IUFillNumber(&MaxPositionN[0], "Steps", "", "%.f", 0, 500000, 0., 10000);
//    IUFillNumberVector(&MaxPositionNP, MaxPositionN, 1, getDeviceName(), "Max Position", "", FOCUS_SETTINGS_TAB, IP_RW,
//                       0, IPS_IDLE);

//    // Sync to a particular position
//    IUFillNumber(&SyncN[0], "Ticks", "", "%.f", 0, 100000, 100., 0.);
//    IUFillNumberVector(&SyncNP, SyncN, 1, getDeviceName(), "Sync", "", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

//    FocusAbsPosN[0].min = SyncN[0].min = 0;
//    FocusAbsPosN[0].max = SyncN[0].max = MaxPositionN[0].value;
//    FocusAbsPosN[0].step = SyncN[0].step = MaxPositionN[0].value / 50.0;
//    FocusAbsPosN[0].value                = 0;

//    FocusRelPosN[0].max   = (FocusAbsPosN[0].max - FocusAbsPosN[0].min) / 2;
//    FocusRelPosN[0].step  = FocusRelPosN[0].max / 100.0;
//    FocusRelPosN[0].value = 100;

    addSimulationControl();

    return true;
}

bool PerfectStarB::updateProperties()
{
    INDI::Focuser::updateProperties();

//    if (isConnected())
//    {
//        defineProperty(&SyncNP);
//        defineProperty(&MaxPositionNP);
//    }
//    else
//    {
//        deleteProperty(SyncNP.name);
//        deleteProperty(MaxPositionNP.name);
//    }

    return true;
}

void PerfectStarB::TimerHit()
{
    if (!isConnected())
        return;

    uint32_t currentTicks = 0;

    bool rc = getPosition(&currentTicks);

    if (rc)
        FocusAbsPosN[0].value = currentTicks;

    getStatus(&status);

    if (FocusAbsPosNP.s == IPS_BUSY || FocusRelPosNP.s == IPS_BUSY)
    {
        if (sim)
        {
            if (FocusAbsPosN[0].value < targetPosition)
                simPosition += 500;
            else
                simPosition -= 500;

            if (std::abs((int64_t)simPosition - (int64_t)targetPosition) < 500)
            {
                FocusAbsPosN[0].value = targetPosition;
                simPosition           = FocusAbsPosN[0].value;
                status                = PS_NOOP;
            }

            FocusAbsPosN[0].value = simPosition;
        }

        if (status == PS_HALT && targetPosition == FocusAbsPosN[0].value)
        {
            if (FocusRelPosNP.s == IPS_BUSY)
            {
                FocusRelPosNP.s = IPS_OK;
                IDSetNumber(&FocusRelPosNP, nullptr);
            }

            FocusAbsPosNP.s = IPS_OK;
            LOG_DEBUG("Focuser reached target position.");
        }
        else if (status == PS_NOOP)
        {
            if (FocusRelPosNP.s == IPS_BUSY)
            {
                FocusRelPosNP.s = IPS_OK;
                IDSetNumber(&FocusRelPosNP, nullptr);
            }

            FocusAbsPosNP.s = IPS_OK;
            LOG_INFO("Focuser reached home position.");
        }
    }

    IDSetNumber(&FocusAbsPosNP, nullptr);

    SetTimer(getCurrentPollingPeriod());
}

bool PerfectStarB::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Max Travel
//        if (strcmp(MaxPositionNP.name, name) == 0)
//        {
//            IUUpdateNumber(&MaxPositionNP, values, names, n);

//            if (MaxPositionN[0].value > 0)
//            {
//                FocusAbsPosN[0].min = SyncN[0].min = 0;
//                FocusAbsPosN[0].max = SyncN[0].max = MaxPositionN[0].value;
//                FocusAbsPosN[0].step = SyncN[0].step = MaxPositionN[0].value / 50.0;

//                FocusRelPosN[0].max  = (FocusAbsPosN[0].max - FocusAbsPosN[0].min) / 2;
//                FocusRelPosN[0].step = FocusRelPosN[0].max / 100.0;
//                FocusRelPosN[0].min  = 0;

//                IUUpdateMinMax(&FocusAbsPosNP);
//                IUUpdateMinMax(&FocusRelPosNP);
//                IUUpdateMinMax(&SyncNP);

//                LOGF_INFO("Focuser absolute limits: min (%g) max (%g)", FocusAbsPosN[0].min,
//                       FocusAbsPosN[0].max);
//            }

//            MaxPositionNP.s = IPS_OK;
//            IDSetNumber(&MaxPositionNP, nullptr);
//            return true;
//        }

        // Sync
//        if (strcmp(SyncNP.name, name) == 0)
//        {
//            IUUpdateNumber(&SyncNP, values, names, n);
//            if (!sync(SyncN[0].value))
//                SyncNP.s = IPS_ALERT;
//            else
//                SyncNP.s = IPS_OK;

//            IDSetNumber(&SyncNP, nullptr);
//            return true;
//        }
    }

    return INDI::Focuser::ISNewNumber(dev, name, values, names, n);
}

IPState PerfectStarB::MoveAbsFocuser(uint32_t targetTicks)
{
    bool rc = setPosition(targetTicks);

    if (!rc)
        return IPS_ALERT;

    targetPosition = targetTicks;

    rc = setStatus(PS_GOTO);

    if (!rc)
        return IPS_ALERT;

    FocusAbsPosNP.s = IPS_BUSY;

    return IPS_BUSY;
}

IPState PerfectStarB::MoveRelFocuser(FocusDirection dir, uint32_t ticks)
{
    uint32_t finalTicks = FocusAbsPosN[0].value + (ticks * (dir == FOCUS_INWARD ? -1 : 1));

    return MoveAbsFocuser(finalTicks);
}

bool PerfectStarB::setPosition(uint32_t ticks)
{
    int rc = 0;
    unsigned char command[3];
    unsigned char response[3];

    // 20 bit resolution position. 4 high bits + 16 lower bits

    // Send 4 high bits first
    command[0] = 0x28;
    command[1] = (ticks & 0x40000) >> 16;

    LOGF_DEBUG("Set Position (%ld)", ticks);
    LOGF_DEBUG("CMD (%02X %02X)", command[0], command[1]);

    if (sim)
        rc = 2;
    else
        rc = hid_write(handle, command, 2);

    if (rc < 0)
    {
        LOGF_ERROR("setPosition: Error writing to device (%s)", hid_error(handle));
        return false;
    }

    if (sim)
    {
        rc          = 2;
        response[0] = 0x28;
        response[1] = command[1];
    }
    else
        rc = hid_read_timeout(handle, response, 2, PERFECTSTAR_TIMEOUT);

    if (rc < 0)
    {
        LOGF_ERROR("setPosition: Error reading from device (%s)", hid_error(handle));
        return false;
    }

    LOGF_DEBUG("RES (%02X %02X)", response[0], response[1]);

    // Send lower 16 bit
    command[0] = 0x20;
    // Low Byte
    command[1] = ticks & 0xFF;
    // High Byte
    command[2] = (ticks & 0xFF00) >> 8;

    LOGF_DEBUG("CMD (%02X %02X %02X)", command[0], command[1], command[2]);

    if (sim)
        rc = 3;
    else
        rc = hid_write(handle, command, 3);

    if (rc < 0)
    {
        LOGF_ERROR("setPosition: Error writing to device (%s)", hid_error(handle));
        return false;
    }

    if (sim)
    {
        rc          = 3;
        response[0] = command[0];
        response[1] = command[1];
        response[2] = command[2];
    }
    else
        rc = hid_read_timeout(handle, response, 3, PERFECTSTAR_TIMEOUT);

    if (rc < 0)
    {
        LOGF_ERROR("setPosition: Error reading from device (%s)", hid_error(handle));
        return false;
    }

    LOGF_DEBUG("RES (%02X %02X %02X)", response[0], response[1], response[2]);

    targetPosition = ticks;

    // TODO add checking later
    return true;
}

bool PerfectStarB::getPosition(uint32_t *ticks)
{
    int rc       = 0;
    uint32_t pos = 0;
    unsigned char command[1];
    unsigned char response[3];

    // 20 bit resolution position. 4 high bits + 16 lower bits

    // Get 4 high bits first
    command[0] = 0x29;

    LOG_DEBUG("Get Position (High 4 bits)");
    LOGF_DEBUG("CMD (%02X)", command[0]);

    if (sim)
        rc = 2;
    else
        rc = hid_write(handle, command, 1);

    if (rc < 0)
    {
        LOGF_ERROR("getPosition: Error writing to device (%s)", hid_error(handle));
        return false;
    }

    if (sim)
    {
        rc          = 2;
        response[0] = command[0];
        response[1] = simPosition >> 16;
    }
    else
        rc = hid_read_timeout(handle, response, 2, PERFECTSTAR_TIMEOUT);

    if (rc < 0)
    {
        LOGF_ERROR("getPosition: Error reading from device (%s)", hid_error(handle));
        return false;
    }

    LOGF_DEBUG("RES (%02X %02X)", response[0], response[1]);

    // Store 4 high bits part of a 20 bit number
    pos = response[1] << 16;

    // Get 16 lower bits
    command[0] = 0x21;

    LOG_DEBUG("Get Position (Lower 16 bits)");
    LOGF_DEBUG("CMD (%02X)", command[0]);

    if (sim)
        rc = 1;
    else
        rc = hid_write(handle, command, 1);

    if (rc < 0)
    {
        LOGF_ERROR("getPosition: Error writing to device (%s)", hid_error(handle));
        return false;
    }

    if (sim)
    {
        rc          = 3;
        response[0] = command[0];
        response[1] = simPosition & 0xFF;
        response[2] = (simPosition & 0xFF00) >> 8;
    }
    else
        rc = hid_read_timeout(handle, response, 3, PERFECTSTAR_TIMEOUT);

    if (rc < 0)
    {
        LOGF_ERROR("getPosition: Error reading from device (%s)", hid_error(handle));
        return false;
    }

    LOGF_DEBUG("RES (%02X %02X %02X)", response[0], response[1], response[2]);

    // Res[1] is lower byte and Res[2] is high byte. Combine them and add them to ticks.
    pos |= response[1] | response[2] << 8;

    *ticks = pos;

    LOGF_DEBUG("Position: %ld", pos);

    return true;
}

bool PerfectStarB::setStatus(PS_STATUS targetStatus)
{
    int rc = 0;
    unsigned char command[2];
    unsigned char response[3];

    command[0] = 0x10;
    command[1] = (targetStatus == PS_HALT) ? 0xFF : targetStatus;

    LOGF_DEBUG("CMD (%02X %02X)", command[0], command[1]);

    if (sim)
        rc = 2;
    else
        rc = hid_write(handle, command, 2);

    if (rc < 0)
    {
        LOGF_ERROR("setStatus: Error writing to device (%s)", hid_error(handle));
        return false;
    }

    if (sim)
    {
        rc          = 3;
        response[0] = command[0];
        response[1] = 0;
        response[2] = command[1];
        status      = targetStatus;
        // Convert Goto to either "moving in" or "moving out" status
        if (status == PS_GOTO)
        {
            // Moving in state
            if (targetPosition < FocusAbsPosN[0].value)
                status = PS_IN;
            else
                // Moving out state
                status = PS_OUT;
        }
    }
    else
        rc = hid_read_timeout(handle, response, 3, PERFECTSTAR_TIMEOUT);

    if (rc < 0)
    {
        LOGF_ERROR("setStatus: Error reading from device (%s)", hid_error(handle));
        return false;
    }

    LOGF_DEBUG("RES (%02X %02X %02X)", response[0], response[1], response[2]);

    if (response[1] == 0xFF)
    {
        LOG_ERROR("setStatus: Invalid state change.");
        return false;
    }

    return true;
}

bool PerfectStarB::getStatus(PS_STATUS *currentStatus)
{
    int rc = 0;
    unsigned char command[1];
    unsigned char response[2];

    command[0] = 0x11;

    LOGF_DEBUG("CMD (%02X)", command[0]);

    if (sim)
        rc = 1;
    else
        rc = hid_write(handle, command, 1);

    if (rc < 0)
    {
        LOGF_ERROR("getStatus: Error writing to device (%s)", hid_error(handle));
        return false;
    }

    if (sim)
    {
        rc          = 2;
        response[0] = command[0];
        response[1] = status;
        // Halt/SetPos is state = 0 "not moving".
        if (response[1] == PS_HALT || response[1] == PS_SETPOS)
            response[1] = 0;
    }
    else
        rc = hid_read_timeout(handle, response, 2, PERFECTSTAR_TIMEOUT);

    if (rc < 0)
    {
        LOGF_ERROR("getStatus: Error reading from device (%s)", hid_error(handle));
        return false;
    }

    LOGF_DEBUG("RES (%02X %02X)", response[0], response[1]);

    switch (response[1])
    {
        case 0:
            *currentStatus = PS_HALT;
            LOG_DEBUG("State: Not moving.");
            break;

        case 1:
            *currentStatus = PS_IN;
            LOG_DEBUG("State: Moving in.");
            break;

        case 3:
            *currentStatus = PS_GOTO;
            LOG_DEBUG("State: Goto.");
            break;

        case 2:
            *currentStatus = PS_OUT;
            LOG_DEBUG("State: Moving out.");
            break;

        case 5:
            *currentStatus = PS_LOCKED;
            LOG_DEBUG("State: Locked.");
            break;

        default:
            LOGF_WARN("Warning: Unknown status (%d)", response[1]);
            return false;
            break;
    }

    return true;
}

bool PerfectStarB::AbortFocuser()
{
    return setStatus(PS_HALT);
}

//bool PerfectStarB::sync(uint32_t ticks)
bool PerfectStarB::SyncFocuser(uint32_t ticks)
{
    bool rc = setPosition(ticks);

    if (!rc)
        return false;

    simPosition = ticks;

    rc = setStatus(PS_SETPOS);

    return rc;
}

//bool PerfectStarB::saveConfigItems(FILE *fp)
//{
//    INDI::Focuser::saveConfigItems(fp);

//    IUSaveConfigNumber(fp, &MaxPositionNP);

//    return true;
//}

