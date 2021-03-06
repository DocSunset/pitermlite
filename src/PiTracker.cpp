// PiTracker.cpp


/*
Polhemus Tracker Terminal (PiTerm) version 1.0.3 -- Terminal Interface to Polhemus Trackers: Fastrak, Patriot, and Liberty
Copyright  ©  2009-2011  Polhemus, Inc.

This file is part of Tracker Terminal.

Tracker Terminal is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Tracker Terminal is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tracker Terminal.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************


*/

#include <termios.h>
#include "PiTracker.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdio.h>
#include <thread>

#define FT_INT   0x84
#define FT_PID   0x0002

PiTracker::PiTracker()
{
  m_handle = NULL;
  m_rs232Port = -1;
  m_cnxType = NO_CNX;
  m_bCloseUsbLibrary = 0;
  m_FtContUsb = 0;
  m_isFt = 0;
  m_lastFtCont = 0;
}

PiTracker::~PiTracker()
{
  if (m_cnxType != NO_CNX)
    CloseTrk();
}

int PiTracker::UsbConnect(USB_PARAMS p)
{
  return UsbConnect(p.vid, p.pid, p.writeEp, p.readEp);
}

int PiTracker::UsbConnect(int Vid, int Pid, int writeEp, int readEp)
{
  int r = libusb_init(NULL);
  if (r < 0) 
  {
    fprintf(stderr, "libusb failed initialization attempt\n");
    return -1;
  }
  m_bCloseUsbLibrary = 1;
  if (Pid == FT_PID) m_isFt = 1;

  m_usbVid = Vid;
  m_usbPid = Pid;
  m_usbWriteEp = writeEp;
  m_usbReadEp = readEp;

  m_handle = libusb_open_device_with_vid_pid(NULL, m_usbVid, m_usbPid);
  if (!m_handle) return -1;

  m_cnxType = USB_CNX;

  return 0;
}

int PiTracker::Rs232Connect(const char* port, int baud/*=115200*/)
{
  int br; 
  struct termios newAtt;

  switch (baud)
  {
  case 9600:
    br = B9600;
    break;
  case 19200:
    br = B19200;
    break;
  case 38400:
    br = B38400;
    break;
  case 57600:
    br = B57600;
    break;
  case 115200:
    br = B115200;
    break;
  default:
    fprintf(stderr, "Baud value not available in this application, defaulting to 115200\n");
    br = B115200;
  }

  m_rs232Port = open(port, O_RDWR|O_NOCTTY|O_NDELAY);
  if (m_rs232Port == -1)
  {
    fprintf(stderr, "Error connecting to tracker.\nDo you have necessary permissions to access %s?\n", port);
    return -1;
  }

  // set up terminal for raw data
  tcgetattr(m_rs232Port, &m_initialAtt);      // save this to restore later
  newAtt = m_initialAtt;
  cfmakeraw(&newAtt);
  cfsetspeed(&newAtt, br);           // set baud
  if (tcsetattr(m_rs232Port, TCSANOW, &newAtt))
  {
    printf("Error setting terminal attributes\n");
    close(m_rs232Port);
    return -2;
  }

  m_cnxType = RS232_CNX;
  return 0;
}

int PiTracker::WriteTrkData(void* data, int len)
{
  int bw = 0;

  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_cnxType == USB_CNX) bw = WriteUsbData(data, len);
  else if (m_cnxType == RS232_CNX) bw = WriteRs232Data(data, len);
  else fprintf(stderr, "No Connection\n");

  return bw;
}

int PiTracker::ReadTrkData(void* buf, int maxLen)
{
  int br = 0;

  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_cnxType == USB_CNX) br = ReadUsbData(buf, maxLen);
  else if (m_cnxType == RS232_CNX) br = ReadRs232Data(buf, maxLen);
  else fprintf(stderr, "No Connection\n");

  return br;
}


void PiTracker::CloseTrk()
{
  if (m_cnxType == USB_CNX)
  {
    libusb_close(m_handle);
    m_handle = NULL;
  } 
  else if (m_cnxType == RS232_CNX)
  {
    // return attributes to initial settings
    tcsetattr(m_rs232Port, TCSANOW, &m_initialAtt);
    close(m_rs232Port);
    m_rs232Port = -1;
  } 
  else
    fprintf(stderr, "No connection to close\n");

  if (m_bCloseUsbLibrary)
  {
    libusb_exit(NULL);
    m_bCloseUsbLibrary = 0;
  }

  m_cnxType = NO_CNX;
}


int PiTracker::WriteUsbData(void* data, int len)
{
  int bw;

  // special case for Fastrak, it puts its continuous data over an interrupt endpoint
  if (((*(char*)data) == 'C') && m_isFt) m_FtContUsb = 1;
  else if (((*(char*)data) == 'c') && m_isFt) m_lastFtCont = 1;

  libusb_bulk_transfer(m_handle, m_usbWriteEp, (BYTE*)data, len, &bw, 50);
  return bw;
}



int PiTracker::WriteRs232Data(void* data, int len)
{
  return write(m_rs232Port, data, len);
}



int PiTracker::ReadUsbData(void* buf, int maxlen)
{
  int br;

  if (m_FtContUsb)
  {
    // Fastrak's interrupt endpoint
    libusb_interrupt_transfer(m_handle, FT_INT, (BYTE*)buf, 
                              maxlen, &br, 50); 
    if (m_lastFtCont && (br == 0)) m_FtContUsb = m_lastFtCont = 0;
  } 
  else libusb_bulk_transfer(m_handle, m_usbReadEp, (BYTE*)buf, 
                            maxlen, &br, 50);

  return br;
}

int PiTracker::ReadRs232Data(void* buf, int maxLen)
{
  return read(m_rs232Port, (BYTE*)buf, maxLen);
}

int PiTracker::GetCnxType()
{
  return m_cnxType;
}
