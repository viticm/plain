#include "pf/net/stream/input.h"
#include "pf/net/stream/output.h"
#include "pf/net/packet/dynamic.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/basic/io.tcc"
#include "pf/sys/assert.h"
#include "pf/basic/logger.h"
#include "pf/net/protocol/basic.h"

namespace pf_net {

namespace protocol {

Basic::Basic() {
  //do nothing.
}

Basic::~Basic() {
  //do nothing.
}

bool Basic::command(connection::Basic *connection, uint16_t count) {
  bool result = false;
  char packetheader[NET_PACKET_HEADERSIZE + 1] = {0};
  uint16_t packetid = 0;
  stream::Input *istream = &connection->istream();
  uint32_t packetcheck, packetsize, packetindex;
  packet::Interface *packet = nullptr;
  //if (isdisconnect()) return true; leave this to connection.
  try {
    uint32_t i;
    for (i = 0; i < count; ++i) {
      memset(packetheader, 0, sizeof(packetheader));
      if (!istream) return true;
      if (!istream->peek(&packetheader[0], NET_PACKET_HEADERSIZE)) {
        //数据不能填充消息头
        break;
      }
      memcpy(&packetid, &packetheader[0], sizeof(packetid));
      memcpy(&packetcheck, 
             &packetheader[sizeof(packetid)], 
             sizeof(packetcheck));
      packetsize = NET_PACKET_GETLENGTH(packetcheck);
      packetindex = NET_PACKET_GETINDEX(packetcheck);
      if (!NET_PACKET_FACTORYMANAGER_POINTER->
          is_valid_packet_id(packetid) &&
          !NET_PACKET_FACTORYMANAGER_POINTER->
          is_valid_dynamic_packet_id(packetid)) {
        pf_basic::io_cerr("packet id error: %d", packetid);
        return false;
      }
      try {
        //check packet length
        if (istream->size() < 
            NET_PACKET_HEADERSIZE + packetsize) {
          //message not receive full
          break;
        }
        //check packet size
        if (!NET_PACKET_FACTORYMANAGER_POINTER->
            is_valid_dynamic_packet_id(packetid)) {
          if (packetsize > 
            NET_PACKET_FACTORYMANAGER_POINTER->packet_max_size(packetid)) {
            char temp[FILENAME_MAX] = {0};
            snprintf(temp, 
                     sizeof(temp) - 1, 
                     "packet size error, packetid = %d", 
                     packetid);
            AssertEx(false, temp);
            return false;
          }
        }
        //create packet
        packet = NET_PACKET_FACTORYMANAGER_POINTER->packet_create(packetid);
        if (nullptr == packet) return false;

        //packet info
        packet->set_index(static_cast<int8_t>(packetindex));
        packet->set_id(packetid);
        packet->set_size(packetsize);
        
        //read packet
        result = istream->skip(NET_PACKET_HEADERSIZE);
        result = result ? packet->read(*istream) : result;
        if (false == result) {
          NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
          return result;
        }
        bool needremove = true;
        bool exception = false;
        uint32_t executestatus = 0;
        try {
          //connection->resetkick();
          try {
            executestatus = packet->execute(connection);
          } catch(...) {
            SaveErrorLog();
            executestatus = kPacketExecuteStatusError;
          }
          if (kPacketExecuteStatusError == executestatus) {
            if (packet) 
              NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
            return false;
          } else if (kPacketExecuteStatusBreak == executestatus) {
            if (packet) 
              NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
            break;
          } else if (kPacketExecuteStatusContinue == executestatus) {
            //continue read last packet
          } else if (kPacketExecuteStatusNotRemove == executestatus) {
            needremove = false;
          } else if (kPacketExecuteStatusNotRemoveError == executestatus) {
            return false;
          } else {
            //unknown status
          }
        } catch(...) {
          SaveErrorLog();
          exception = true;
        }
        if (packet && needremove) { 
          NET_PACKET_FACTORYMANAGER_POINTER->packet_remove(packet);
        }
        if (exception) return false;
      } catch(...) {
        SaveErrorLog();
        return false;
      }
    }
  } catch(...) {
    SaveErrorLog();
    return false;
  }
  return true;
}

bool Basic::compress(connection::Basic * connection, 
                     char *uncompress_buffer, 
                     char *compress_buffer) {
  stream::Input &istream = connection->istream();
  //stream::Output &ostream = connection->ostream();
  stream::Input &istream_compress = connection->istream_compress();
  //Write the compress input stream decode result to the normal.
  pf_util::compressor::Assistant *assistant = nullptr;
  assistant = istream.getcompressor()->getassistant();    
  if (!assistant->isenable() || is_null(&istream_compress)) return false;
  uint16_t compressheader = 0;
  char packetheader[NET_PACKET_HEADERSIZE] = {0};
  uint16_t packetid = 0;
  uint32_t packetcheck = 0;
  uint32_t packetsize = 0;
  uint32_t size = 0;
  uint32_t result = 0;
  if (is_null(uncompress_buffer) || is_null(compress_buffer)) {
    Assert(false);
    return false;
  }
  memset(uncompress_buffer, 0, NET_CONNECTION_UNCOMPRESS_BUFFER_SIZE);
  memset(compress_buffer, 0, NET_CONNECTION_COMPRESS_BUFFER_SIZE);
  do {
    if (!istream_compress.peek(
          reinterpret_cast<char *>(&compressheader), 
          sizeof(compressheader))) {
      break;
    }
    if (static_cast<int16_t>(compressheader) < 0) {
      compressheader &= 0x7FFF;
      uint32_t totalsize = sizeof(compressheader) + compressheader;
      if (size < totalsize) break;
      result = istream_compress.read(uncompress_buffer, totalsize);
      if (0 == result) return false;
      uint32_t outsize = 0;
      bool _result = istream.getcompressor()->decompress(
          uncompress_buffer, compressheader, compress_buffer, outsize);
      if (!_result) {
        SLOW_ERRORLOG(
            NET_MODULENAME,
            "[net.protocol] (Basic::compress)"
            " istream->getcompressor()->decompress fail");
        return false;
      }
      if (istream.encrypt_isenable()) {
        istream.getencryptor()
          ->decrypt(compress_buffer, compress_buffer, outsize);
      }
      result = istream.write(compress_buffer, outsize);
      if (result != outsize) {
        SLOW_ERRORLOG(
            NET_MODULENAME,
            "[net.protocol] (Basic::compress)"
            " istream.write fail result: %d, outsize: %d",
            result,
            outsize);
        return false;
      }
    } else {
      if (!istream_compress.peek(&packetheader[0], sizeof(packetheader)))
        break;
      memcpy(&packetid, &packetheader[0], sizeof(packetid));
      memcpy(&packetcheck, 
             &packetheader[sizeof(packetid)], 
             sizeof(packetcheck));
      if (!NET_PACKET_FACTORYMANAGER_POINTER->is_valid_packet_id(packetid) &&
          !NET_PACKET_FACTORYMANAGER_POINTER
          ->is_valid_dynamic_packet_id(packetid)) {
        SLOW_ERRORLOG(
            NET_MODULENAME,
            "[net.connection] (Basic::process_compressinput)"
            " packetid not valid id: %d",
            packetid);
        return false;
      }
      packetsize = NET_PACKET_GETLENGTH(packetcheck);
      size = istream_compress.size();
      if (!NET_PACKET_FACTORYMANAGER_POINTER
          ->is_valid_dynamic_packet_id(packetid)) {
        uint32_t sizemax = 
          NET_PACKET_FACTORYMANAGER_POINTER->packet_max_size(packetid);
        if (packetsize > sizemax) {
          SLOW_ERRORLOG(
              NET_MODULENAME,
              "[net.connection] (Basic::process_compressinput)"
              " packet size more than max size(%d, %d, %d)",
              packetid,
              packetsize,
              sizemax);
          return false;
        }
      }
      //Waiting for full.
      uint32_t totalsize = NET_PACKET_HEADERSIZE + packetsize;
      if (size < totalsize) break;

      //Read it.
      result = istream_compress.read(uncompress_buffer, totalsize);
      if (0 == result) return false;
      result = istream.write(uncompress_buffer, totalsize);
      if (result != totalsize) {
        SLOW_ERRORLOG(
            NET_MODULENAME,
            "[net.protocol] (Basic::compress)"
            " read write data size not equal(%d, %d)",
            totalsize,
            result);
        return false;
      }
      if (NET_PACKET_FACTORYMANAGER_POINTER->is_encrypt_packet_id(packetid))
        break;
    }
  } while(true);
  return true;
}

bool Basic::send(connection::Basic * connection, packet::Interface *packet) {
  bool result = false;
  stream::Output &ostream = connection->ostream();
  if (&ostream) {
    if (!ostream.use(NET_PACKET_HEADERSIZE + packet->size())) {
      return false;
    }
    packet->set_index(connection->packet_index());
    uint32_t before_writesize = ostream.size();
    uint16_t packetid = packet->get_id();

    uint32_t packetcheck{0}; //index and size(if diffrent then have error) 
    ostream.write(reinterpret_cast<const char *>(&packetid), sizeof(packetid));
    uint32_t packetsize = packet->size();
    uint32_t packetindex = packet->get_index();
    NET_PACKET_SETINDEX(packetcheck, packetindex);
    NET_PACKET_SETLENGTH(packetcheck, packetsize);
    ostream.write(reinterpret_cast<const char *>(&packetcheck), 
                  sizeof(packetcheck));
    result = packet->write(ostream);
    Assert(result);
    uint32_t after_writesize = ostream.size();
    if (packet->size() != 
        after_writesize - before_writesize - NET_PACKET_HEADERSIZE) {
      FAST_ERRORLOG(NET_MODULENAME,
                    "[net.protocol] (Basic::send) size error,"
                    " id = %d(write: %d, should: %d)",
                    packet->get_id(),
                    after_writesize - before_writesize - 6,
                    packet->size());
      result = false;
    }
  }
  return result;
}

} //namespace protocol

} //namespace pf_net
